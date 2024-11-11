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

#pragma once

#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QGSettings>
#include <QObject>
#include <memory>

class Power : public QObject
{
    Q_OBJECT
public:
    virtual ~Power();

    static std::shared_ptr<Power> getDefault();

    // 可以使用的TV数
    uint32_t getNtvsTotal();
    // 图形已经使用的TV数
    uint32_t getGraphicalNtvs();

    //待机
    bool suspend();
    //休眠
    bool hibernate();
    bool shutdown();
    bool reboot();
    bool logout();
    bool switchUser();
    bool lockScreen();

    bool canSuspend();
    bool canHibernate();
    bool canShutdown();
    bool canReboot();
    bool canLogout();
    bool canSwitchUser();
    bool canLockScreen();

signals:

private:
    explicit Power(QObject* parent = nullptr);

private:
    static std::shared_ptr<Power> m_instance;

    //gsettings
    QGSettings* m_gsettings;

    //dbus相关
    QDBusInterface* m_login1Proxy;
    QDBusInterface* m_sessionManagerProxy;
    QDBusInterface* m_seatManagerProxy;
};
