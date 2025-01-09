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

#pragma once

#include <QDBusConnection>
#include <QObject>

#define DBusWatcher DBusServiceWatcher::getInstance()

class QDBusServiceWatcher;
class DBusServiceWatcher : public QObject
{
    Q_OBJECT
public:
    static DBusServiceWatcher& getInstance();

    void AddService(const QString& newService, QDBusConnection::BusType type);
    void removeService(const QString& service);

private:
    explicit DBusServiceWatcher(QObject* parent = nullptr);

signals:
    void serviceOwnerChanged(const QString& serviceName, const QString& oldOwner, const QString& newOwner);

private:
    QDBusServiceWatcher* m_systemServiceWatcher;
    QDBusServiceWatcher* m_sessionServiceWatcher;
};
