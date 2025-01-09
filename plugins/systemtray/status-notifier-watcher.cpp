/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
 * kiran-shell is licensed under Mulan PSL v2.
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

#include "lib/common/dbus-service-watcher.h"
#include "status-notifier-watcher.h"
#include "statusnotifierwatcherinterface.h"

#define SERVICE_NAME QLatin1String("org.kde.StatusNotifierWatcher")
#define WATCHER_PATH QLatin1String("/StatusNotifierWatcher")

StatusNotifierWatcher::StatusNotifierWatcher(QObject *parent)
    : QObject{parent}, m_xembedSniProxy(nullptr)
{
    registerServer();

    connect(&DBusWatcher, &DBusServiceWatcher::serviceOwnerChanged, this, &StatusNotifierWatcher::serviceOwnerChanged);
    DBusWatcher.AddService(SERVICE_NAME, QDBusConnection::SessionBus);
}

StatusNotifierWatcher::~StatusNotifierWatcher()
{
    killXembedSniProxy();

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
    // 监控 org.kde.StatusNotifierWatcher 注册者变动，若其他进程注销，本服务接替注册
    if (SERVICE_NAME != service)
    {
        return;
    }

    // Note that this signal is also emitted whenever the serviceName service was registered or unregistered.
    // If it was registered, oldOwner will contain an empty string,
    // whereas if it was unregistered, newOwner will contain an empty string

    KLOG_INFO() << "Service" << service << "status change, old owner:" << oldOwner << "new:" << newOwner;

    // 能接收到这个信号，说明有其他面板程序注册了服务，如：kiran-applet
    if (newOwner.isEmpty())
    {
        KLOG_INFO() << "other program is unregistered, start to register" << SERVICE_NAME;

        registerServer();
    }
}

void StatusNotifierWatcher::serviceUnregistered(const QString &service)
{
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
    // 任意时间都将有且只有一个 org.freedesktop.StatusNotifierWatcher 服务实例在会话中被注册
    // 如果其他程序注册了（如kiran-applet），这里会注册失败
    if (!dbus.registerService(SERVICE_NAME) ||
        !dbus.registerObject(WATCHER_PATH, this, QDBusConnection::ExportAllContents))
    {
        KLOG_WARNING() << "service register failed:" << SERVICE_NAME << ", because other program is registered earlier";
    }
    else
    {
        KLOG_INFO() << "service register ok:" << SERVICE_NAME;
        // 启动　x转sni　服务
        startXembedSniProxy();
    }
}

void StatusNotifierWatcher::startXembedSniProxy()
{
    killXembedSniProxy();

    m_xembedSniProxy = new QProcess;

    // 将标准输出和标准错误输出合并
    m_xembedSniProxy->setProcessChannelMode(QProcess::MergedChannels);
    m_xembedSniProxy->start("xembedsniproxy", QStringList());
    if (m_xembedSniProxy->waitForStarted())
    {
        KLOG_INFO() << "xembedsniproxy 启动成功";
    }
    else
    {
        KLOG_INFO() << "xembedsniproxy 启动失败，失败信息：" << m_xembedSniProxy->readAll();
        m_xembedSniProxy->deleteLater();
        m_xembedSniProxy = nullptr;
    }
}

void StatusNotifierWatcher::killXembedSniProxy()
{
    if (m_xembedSniProxy)
    {
        if (m_xembedSniProxy->state() == QProcess::Running)
        {
            m_xembedSniProxy->terminate();  // SIGTERM
            if (!m_xembedSniProxy->waitForFinished(3000))
            {
                m_xembedSniProxy->kill();  // SIGKILL
                KLOG_INFO() << "xembedsniproxy 进程被强制终止。";
            }
            else
            {
                KLOG_INFO() << "xembedsniproxy 进程已成功终止。";
            }
        }

        m_xembedSniProxy->deleteLater();  // 清理进程
        m_xembedSniProxy = nullptr;       // 防止再次使用已删除的指针
    }
    else
    {
        // 调试模式下，停止主进程，子进程会变孤儿
        QProcess process;
        process.start("pkill", {"xembedsniproxy"});
        if (process.waitForFinished())
        {
            KLOG_INFO() << "xembedsniproxy 进程已被终止";
        }
        else
        {
            KLOG_INFO() << "xembedsniproxy 无法终止";
        }
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

    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(service).value())
    {
        m_registeredServices.append(itemId);
        emit StatusNotifierItemRegistered(itemId);
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
