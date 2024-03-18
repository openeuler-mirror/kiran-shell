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

#pragma once

#include <QDBusContext>
#include <QObject>
#include <QStringList>

#include "statusnotifierwatcherinterface.h"

class QDBusServiceWatcher;

class StatusNotifierWatcher : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.StatusNotifierWatcher")
    Q_PROPERTY(QStringList RegisteredStatusNotifierItems READ RegisteredStatusNotifierItems)
    Q_PROPERTY(bool IsStatusNotifierHostRegistered READ IsStatusNotifierHostRegistered)
    Q_PROPERTY(int ProtocolVersion READ ProtocolVersion)

public:
    StatusNotifierWatcher(QObject *parent = 0);
    ~StatusNotifierWatcher() override;

public:
    //Properties
    //StatusNotifierItem 的所有已注册实例的列表
    QStringList RegisteredStatusNotifierItems() const;
    //如果至少有一个 StatusNotifierHost 已向名为 RegisterStatusNotifierHost 的部分注册并且当前正在运行，则为真
    bool IsStatusNotifierHostRegistered() const;
    //StatusNotifierWatcher 实例实现的协议版本
    int ProtocolVersion() const;

public Q_SLOTS:
    //Methods
    //将 StatusNotifierItem 以其在会话总线上的全名形式注册到 StatusNotifierWatcher 中，例如 org.freedesktop.StatusNotifierItem-4077-1
    void RegisterStatusNotifierItem(const QString &service);
    //将 StatusNotifierHost 以其在会话总线上的全名形式注册到 StatusNotifierWatcher 中，例如 org.freedesktop.StatusNotifierHost-4005
    void RegisterStatusNotifierHost(const QString &service);

Q_SIGNALS:
    //SIGNALS
    void StatusNotifierItemRegistered(const QString &service);
    void StatusNotifierItemUnregistered(const QString &service);
    void StatusNotifierHostRegistered();

private:
    void serviceOwnerChanged(const QString &serviceName, const QString &oldOwner, const QString &newOwner);
    void serviceUnregistered(const QString &service);
    void serviceRegistered(const QString &service);

    void registerServer();

private:
    QDBusServiceWatcher *m_serviceWatcher;

    QStringList m_registeredServices;
};
