/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
 * kiran-session-manager is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     yangfeng <yangfeng@kylinsec.com.cn>
 */

#include <qt5-log-i.h>
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDBusServiceWatcher>

#include "status-notifier-watcher.h"
#include "statusnotifierwatcherinterface.h"

#define SERVICE_NAME QLatin1String("org.kde.StatusNotifierWatcher")
#define WATCHER_PATH QLatin1String("/StatusNotifierWatcher")
#define FREEDESKTOP_PROPERTIES QLatin1String("org.freedesktop.DBus.Properties")

StatusNotifierWatcher::StatusNotifierWatcher(QObject *parent)
    : QObject{parent}
{
    registerServer();
    m_serviceWatcher = new QDBusServiceWatcher(SERVICE_NAME, QDBusConnection::sessionBus(), QDBusServiceWatcher::WatchForOwnerChange, this);
    connect(m_serviceWatcher, &QDBusServiceWatcher::serviceOwnerChanged, this, &StatusNotifierWatcher::serviceOwnerChanged);
}

StatusNotifierWatcher::~StatusNotifierWatcher()
{
    QDBusConnection::sessionBus().unregisterService(SERVICE_NAME);
}

bool StatusNotifierWatcher::IsStatusNotifierHostRegistered() const
{
    // 返回true，代表有显示客户端，即本程序
    // 如果服务不依赖于底部面板，单独一个进程运行，则需要管理Host
    return true;
}

int StatusNotifierWatcher::ProtocolVersion() const
{
    return 0;
}

void StatusNotifierWatcher::serviceOwnerChanged(const QString &service, const QString &oldOwner, const QString &newOwner)
{
    //Note that this signal is also emitted whenever the serviceName service was registered or unregistered.
    //If it was registered, oldOwner will contain an empty string,
    //whereas if it was unregistered, newOwner will contain an empty string

    KLOG_INFO() << "Service" << service << "status change, old owner:" << oldOwner << "new:" << newOwner;

    // 能接收到这个信号，说明有其他面板程序注册了服务，如：kiran-panel
    // 其他面板程序程序退出，本服务接替注册
    if (newOwner.isEmpty())
    {
        KLOG_INFO() << "other program is unregistered, start to register" << SERVICE_NAME;

        registerServer();
    }
}

void StatusNotifierWatcher::serviceUnregistered(const QString &service)
{
    m_serviceWatcher->removeWatchedService(service);

    QString path = service + QLatin1Char('/');
    QStringList::Iterator it = m_registeredServices.begin();
    while (it != m_registeredServices.end())
    {
        if (it->startsWith(path))
        {
            QString name = *it;
            it = m_registeredServices.erase(it);
            emit StatusNotifierItemUnregistered(name);
        }
        else
        {
            ++it;
        }
    }
}

void StatusNotifierWatcher::serviceRegistered(const QString &service)
{
}

void StatusNotifierWatcher::registerServer()
{
    QDBusConnection dbus = QDBusConnection::sessionBus();
    // 注册服务
    // 注册对象，并把函数、信号导出为object的method、signal、property
    if (!dbus.registerService(SERVICE_NAME) ||
        !dbus.registerObject(WATCHER_PATH, this, QDBusConnection::ExportAllContents))
    {
        KLOG_WARNING() << "service register failed:" << SERVICE_NAME << ", because other program is registered earlier";
    }
    else
    {
        KLOG_INFO() << "service register ok:" << SERVICE_NAME;
    }
}

void StatusNotifierWatcher::RegisterStatusNotifierItem(const QString &service)
{
    KLOG_INFO() << "StatusNotifierWatcher::RegisterStatusNotifierItem" << service;

    QString itemId = service + "/StatusNotifierItem";
    if (m_registeredServices.contains(itemId))
    {
        return;
    }

    m_serviceWatcher->addWatchedService(service);
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(service).value())
    {
        m_registeredServices.append(itemId);
        emit StatusNotifierItemRegistered(itemId);
    }
    else
    {
        m_serviceWatcher->removeWatchedService(service);
    }
}

void StatusNotifierWatcher::RegisterStatusNotifierHost(const QString &service)
{
    // 不需要管理
    KLOG_INFO() << "StatusNotifierWatcher::RegisterStatusNotifierHost" << service;
}

QStringList StatusNotifierWatcher::RegisteredStatusNotifierItems() const
{
    return m_registeredServices;
}
