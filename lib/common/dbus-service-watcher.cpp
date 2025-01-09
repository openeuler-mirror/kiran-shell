/**
 * Copyright (c) 2024 ~ 2025 KylinSec Co., Ltd.
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

#include <QDBusServiceWatcher>

#include "dbus-service-watcher.h"

DBusServiceWatcher &DBusServiceWatcher::getInstance()
{
    static DBusServiceWatcher instance;
    return instance;
}

void DBusServiceWatcher::AddService(const QString &newService, QDBusConnection::BusType type)
{
    switch (type)
    {
    case QDBusConnection::SessionBus:
    {
        m_sessionServiceWatcher->addWatchedService(newService);
        break;
    }
    case QDBusConnection::SystemBus:
    {
        m_systemServiceWatcher->addWatchedService(newService);
        break;
    }
    default:
        break;
    }
}

void DBusServiceWatcher::removeService(const QString &service)
{
    if (m_systemServiceWatcher->watchedServices().contains(service))
    {
        m_systemServiceWatcher->removeWatchedService(service);
    }

    if (m_sessionServiceWatcher->watchedServices().contains(service))
    {
        m_sessionServiceWatcher->removeWatchedService(service);
    }
}

DBusServiceWatcher::DBusServiceWatcher(QObject *parent)
    : QObject{parent}
{
    m_systemServiceWatcher = new QDBusServiceWatcher(this);
    m_systemServiceWatcher->setConnection(QDBusConnection::systemBus());
    m_systemServiceWatcher->setWatchMode(QDBusServiceWatcher::WatchForOwnerChange);
    connect(m_systemServiceWatcher, &QDBusServiceWatcher::serviceOwnerChanged, this, &DBusServiceWatcher::serviceOwnerChanged);

    m_sessionServiceWatcher = new QDBusServiceWatcher(this);
    m_sessionServiceWatcher->setConnection(QDBusConnection::sessionBus());
    m_sessionServiceWatcher->setWatchMode(QDBusServiceWatcher::WatchForOwnerChange);
    connect(m_sessionServiceWatcher, &QDBusServiceWatcher::serviceOwnerChanged, this, &DBusServiceWatcher::serviceOwnerChanged);
}
