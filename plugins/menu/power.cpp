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
#include <QFile>
#include <QProcess>

#include "power.h"
#include "src/shell/utils.h"

#define SESSION_MANAGER_DBUS "org.gnome.SessionManager"
#define SESSION_MANAGER_PATH "/org/gnome/SessionManager"
#define SESSION_MANAGER_INTERFACE "org.gnome.SessionManager"

#define LOGIN_MANAGER_DBUS "org.freedesktop.login1"
#define LOGIN_MANAGER_PATH "/org/freedesktop/login1"
#define LOGIN_MANAGER_INTERFACE "org.freedesktop.login1.Manager"
#define LOGIN_SESSION_INTERFACE "org.freedesktop.login1.Session"

#define DISPLAY_MANAGER_DBUS "org.freedesktop.DisplayManager"
#define DISPLAY_MANAGER_SEAT_PATH "/org/freedesktop/DisplayManager/Seat0"
#define DISPLAY_MANAGER_INTERFACE "org.freedesktop.DisplayManager.Seat"

#define DBUS_PROXY_TIMEOUT_MSEC 300

#define STARTMENU_LOCKDOWN_SCHEMA_ID "com.kylinsec.kiran.startmenu.lockdown"
#define STARTMENU_LOCKDOWN_KEY_DISABLE_LOCK_SCREEN "disable-lock-screen"
#define STARTMENU_LOCKDOWN_KEY_DISABLE_USER_SWITCHING "disable-user-switching"
#define STARTMENU_LOCKDOWN_KEY_DISABLE_LOG_OUT "disable-log-out"
#define STARTMENU_LOCKDOWN_KEY_DISABLE_SUSPEND "disable-suspend"
#define STARTMENU_LOCKDOWN_KEY_DISABLE_HIBERNATE "disable-hibernate"
#define STARTMENU_LOCKDOWN_KEY_DISABLE_REBOOT "disable-reboot"
#define STARTMENU_LOCKDOWN_KEY_DISABLE_SHUTDOWN "disable-shutdown"

std::shared_ptr<Power> Power::m_instance = nullptr;
std::shared_ptr<Power> Power::getDefault()
{
    if (!m_instance)
    {
        m_instance = std::shared_ptr<Power>(new Power());
    }
    return m_instance;
}

Power::Power(QObject *parent)
    : QObject(parent),
      m_gsettings(nullptr)
{
    try
    {
        m_gsettings = new QGSettings(STARTMENU_LOCKDOWN_SCHEMA_ID);
    }
    catch (...)
    {
        KLOG_WARNING() << "new QGSettings failed:" << STARTMENU_LOCKDOWN_SCHEMA_ID;
    }
    //电源选项D-Bus
    try
    {
        m_login1Proxy = new QDBusInterface(LOGIN_MANAGER_DBUS,
                                           LOGIN_MANAGER_PATH,
                                           LOGIN_MANAGER_INTERFACE,
                                           QDBusConnection::systemBus(),
                                           this);

        m_sessionManagerProxy = new QDBusInterface(SESSION_MANAGER_DBUS,
                                                   SESSION_MANAGER_PATH,
                                                   SESSION_MANAGER_INTERFACE,
                                                   QDBusConnection::sessionBus(),
                                                   this);

        auto xdgSeatObjectPath = qgetenv("XDG_SEAT_PATH");
        m_seatManagerProxy = new QDBusInterface(DISPLAY_MANAGER_DBUS,
                                                xdgSeatObjectPath,
                                                DISPLAY_MANAGER_INTERFACE,
                                                QDBusConnection::systemBus(),
                                                this);

        m_login1Proxy->setTimeout(DBUS_PROXY_TIMEOUT_MSEC);
        m_sessionManagerProxy->setTimeout(DBUS_PROXY_TIMEOUT_MSEC);
        m_seatManagerProxy->setTimeout(DBUS_PROXY_TIMEOUT_MSEC);
    }
    catch (...)
    {
        KLOG_WARNING() << "new QDBusInterface failed";
    }
}

Power::~Power()
{
    if (m_gsettings)
    {
        delete m_gsettings;
        m_gsettings = nullptr;
    }
}

uint32_t Power::getNtvsTotal()
{
    RETURN_VAL_IF_FALSE(m_login1Proxy, 0);

    return m_login1Proxy->property("NAutoVTs").toUInt();
}

uint32_t Power::getGraphicalNtvs()
{
    RETURN_VAL_IF_FALSE(m_seatManagerProxy, 0);

    return m_seatManagerProxy->property("Sessions").toUInt();
}

bool Power::suspend()
{
    RETURN_VAL_IF_FALSE(canSuspend(), false);

    m_login1Proxy->call("Suspend", false);

    return true;
}

bool Power::hibernate()
{
    RETURN_VAL_IF_FALSE(canHibernate(), false);

    m_login1Proxy->call("Hibernate", false);

    return true;
}

bool Power::shutdown()
{
    RETURN_VAL_IF_FALSE(canShutdown(), false);
    RETURN_VAL_IF_FALSE(m_sessionManagerProxy, false);

    m_sessionManagerProxy->call("RequestShutdown");

    return true;
}

bool Power::reboot()
{
    RETURN_VAL_IF_FALSE(canReboot(), false);
    RETURN_VAL_IF_FALSE(m_sessionManagerProxy, false);

    m_sessionManagerProxy->call("RequestReboot");

    return true;
}

bool Power::logout()
{
    RETURN_VAL_IF_FALSE(canLogout(), false);
    RETURN_VAL_IF_FALSE(m_sessionManagerProxy, false);

    quint32 mode = 1;
    QDBusMessage msg = m_sessionManagerProxy->call("Logout", mode);
    if (QDBusMessage::ErrorMessage == msg.type())
    {
        KLOG_WARNING() << msg;
        return false;
    }

    return true;
}

bool Power::switchUser()
{
    RETURN_VAL_IF_FALSE(canSwitchUser(), false);

    QDBusMessage msg = m_seatManagerProxy->call("SwitchToGreeter");
    if (QDBusMessage::ErrorMessage == msg.type())
    {
        KLOG_WARNING() << msg;
        return false;
    }
    return true;
}

bool Power::lockScreen()
{
    RETURN_VAL_IF_FALSE(canLockScreen(), false);

    QProcess::startDetached("kiran-screensaver-command", {"-l"});

    return true;
}

bool Power::canSuspend()
{
    RETURN_VAL_IF_TRUE(m_gsettings->get(STARTMENU_LOCKDOWN_KEY_DISABLE_SUSPEND).toBool(), false);
    RETURN_VAL_IF_FALSE(m_login1Proxy, false);

    QDBusMessage msg = m_login1Proxy->call("CanSuspend");
    if (QDBusMessage::ErrorMessage == msg.type())
    {
        //如果获取失败，就假设其可以待机，由待机操作调用时做检查
        KLOG_WARNING() << msg;
        return true;
    }

    QString data = msg.arguments().first().toString();
    return (data == "yes");
}

bool Power::canHibernate()
{
    RETURN_VAL_IF_TRUE(m_gsettings->get(STARTMENU_LOCKDOWN_KEY_DISABLE_HIBERNATE).toBool(), false);
    RETURN_VAL_IF_FALSE(m_login1Proxy, false);

    QDBusMessage msg = m_login1Proxy->call("CanHibernate");
    if (QDBusMessage::ErrorMessage == msg.type())
    {
        //如果获取失败，就假设其可以休眠，由休眠操作调用时做检查
        KLOG_WARNING() << msg;
        return true;
    }

    QString data = msg.arguments().first().toString();
    return (data == "yes");
}

bool Power::canShutdown()
{
    RETURN_VAL_IF_TRUE(m_gsettings->get(STARTMENU_LOCKDOWN_KEY_DISABLE_SHUTDOWN).toBool(), false);
    RETURN_VAL_IF_FALSE(m_login1Proxy, false);

    QDBusMessage msg = m_login1Proxy->call("CanPowerOff");
    if (QDBusMessage::ErrorMessage == msg.type())
    {
        //如果获取失败，就假设其可以关机，由关机操作调用时做检查
        KLOG_WARNING() << msg;
        return true;
    }

    QString data = msg.arguments().first().toString();
    return (data == "yes");
}

bool Power::canReboot()
{
    RETURN_VAL_IF_TRUE(m_gsettings->get(STARTMENU_LOCKDOWN_KEY_DISABLE_REBOOT).toBool(), false);
    RETURN_VAL_IF_FALSE(m_login1Proxy, false);

    QDBusMessage msg = m_login1Proxy->call("CanReboot");
    if (QDBusMessage::ErrorMessage == msg.type())
    {
        //如果获取失败，就假设其可以重启，由重启操作调用时做检查
        KLOG_WARNING() << msg;
        return true;
    }

    QString data = msg.arguments().first().toString();
    return (data == "yes");
}

bool Power::canLogout()
{
    RETURN_VAL_IF_TRUE(m_gsettings->get(STARTMENU_LOCKDOWN_KEY_DISABLE_LOG_OUT).toBool(), false);

    return true;
}

bool Power::canSwitchUser()
{
    RETURN_VAL_IF_TRUE(m_gsettings->get(STARTMENU_LOCKDOWN_KEY_DISABLE_USER_SWITCHING).toBool(), false);
    RETURN_VAL_IF_FALSE(m_seatManagerProxy, false);

    return m_seatManagerProxy->property("CanSwitch").toBool();
}

bool Power::canLockScreen()
{
    RETURN_VAL_IF_TRUE(m_gsettings->get(STARTMENU_LOCKDOWN_KEY_DISABLE_LOCK_SCREEN).toBool(), false);

    return QFile::exists("/usr/bin/kiran-screensaver-command");
}
