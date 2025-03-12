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
#include <QDBusInterface>
#include <QFile>
#include <QProcess>

#include "free_login1_manager_interface.h"
#include "gnome_session_manager_interface.h"
#include "ks-i.h"
#include "lib/common/logging-category.h"
#include "power.h"
#include "src/shell/utils.h"

#define SESSION_MANAGER_DBUS "org.gnome.SessionManager"
#define SESSION_MANAGER_PATH "/org/gnome/SessionManager"
#define SESSION_MANAGER_INTERFACE "org.gnome.SessionManager"

#define LOGIN_MANAGER_DBUS "org.freedesktop.login1"
#define LOGIN_MANAGER_PATH "/org/freedesktop/login1"
#define LOGIN_MANAGER_INTERFACE "org.freedesktop.login1.Manager"

#define DISPLAY_MANAGER_DBUS "org.freedesktop.DisplayManager"
#define DISPLAY_MANAGER_SEAT_PATH "/org/freedesktop/DisplayManager/Seat0"
#define DISPLAY_MANAGER_INTERFACE "org.freedesktop.DisplayManager.Seat"

#define DBUS_PROXY_TIMEOUT_MSEC 300

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
    : QObject(parent)
{
    m_gsettings = new QGSettings(LOCKDOWN_SCHEMA_ID, "", this);

    // 电源选项D-Bus
    m_freelogin1Manager = new Freelogin1Manager(LOGIN_MANAGER_DBUS,
                                                LOGIN_MANAGER_PATH,
                                                QDBusConnection::systemBus(),
                                                this);
    m_freelogin1Manager->setTimeout(DBUS_PROXY_TIMEOUT_MSEC);

    m_gnomeSessionManager = new GnomeSessionManager(SESSION_MANAGER_DBUS,
                                                    SESSION_MANAGER_PATH,
                                                    QDBusConnection::sessionBus(),
                                                    this);
    m_gnomeSessionManager->setTimeout(DBUS_PROXY_TIMEOUT_MSEC);

    QString xdgSeatPath = qgetenv("XDG_SEAT_PATH");
    if (!xdgSeatPath.isEmpty())
    {
        m_seatManagerProxy = new QDBusInterface(DISPLAY_MANAGER_DBUS,
                                                qgetenv("XDG_SEAT_PATH"),
                                                DISPLAY_MANAGER_INTERFACE,
                                                QDBusConnection::systemBus(),
                                                this);
        m_seatManagerProxy->setTimeout(DBUS_PROXY_TIMEOUT_MSEC);
    }
}

Power::~Power()
{
}

uint32_t Power::getNtvsTotal()
{
    RETURN_VAL_IF_FALSE(m_freelogin1Manager, 0);
    return m_freelogin1Manager->nAutoVTs();
}

uint32_t Power::getGraphicalNtvs()
{
    RETURN_VAL_IF_FALSE(m_seatManagerProxy, 0);
    return m_seatManagerProxy->property("Sessions").toUInt();
}

bool Power::suspend()
{
    RETURN_VAL_IF_FALSE(canSuspend(), false);
    m_freelogin1Manager->Suspend(false);

    return true;
}

bool Power::hibernate()
{
    RETURN_VAL_IF_FALSE(canHibernate(), false);
    m_freelogin1Manager->Hibernate(false);

    return true;
}

bool Power::shutdown()
{
    RETURN_VAL_IF_FALSE(canShutdown(), false);
    RETURN_VAL_IF_FALSE(m_gnomeSessionManager, false);
    m_gnomeSessionManager->RequestShutdown();
    return true;
}

bool Power::reboot()
{
    RETURN_VAL_IF_FALSE(canReboot(), false);
    RETURN_VAL_IF_FALSE(m_gnomeSessionManager, false);
    m_gnomeSessionManager->RequestReboot();
    return true;
}

bool Power::logout()
{
    RETURN_VAL_IF_FALSE(canLogout(), false);
    RETURN_VAL_IF_FALSE(m_gnomeSessionManager, false);

    quint32 mode = 1;
    auto reply = m_gnomeSessionManager->Logout(mode);
    if (reply.isError())
    {
        KLOG_WARNING(LCMenu) << "failed to call Logout" << reply.error();
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
        KLOG_WARNING(LCMenu) << msg;
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
    RETURN_VAL_IF_TRUE(m_gsettings->get(LOCKDOWN_SCHEMA_KEY_DISABLE_SUSPEND).toBool(), false);
    RETURN_VAL_IF_FALSE(m_freelogin1Manager, false);

    auto reply = m_freelogin1Manager->CanSuspend();
    if (reply.isError())
    {
        // 如果获取失败，就假设其可以待机，由待机操作调用时做检查
        KLOG_WARNING(LCMenu) << "failed to call CanSuspend" << reply.error();
        return true;
    }

    QString data = reply.value();
    return (data == "yes");
}

bool Power::canHibernate()
{
    RETURN_VAL_IF_TRUE(m_gsettings->get(LOCKDOWN_SCHEMA_KEY_DISABLE_HIBERNATE).toBool(), false);
    RETURN_VAL_IF_FALSE(m_freelogin1Manager, false);

    auto reply = m_freelogin1Manager->CanHibernate();
    if (reply.isError())
    {
        // 如果获取失败，就假设其可以休眠，由休眠操作调用时做检查
        KLOG_WARNING(LCMenu) << "failed to call CanHibernate" << reply.error();
        return true;
    }

    QString data = reply.value();
    return (data == "yes");
}

bool Power::canShutdown()
{
    RETURN_VAL_IF_TRUE(m_gsettings->get(LOCKDOWN_SCHEMA_KEY_DISABLE_SHUTDOWN).toBool(), false);
    RETURN_VAL_IF_FALSE(m_freelogin1Manager, false);

    auto reply = m_freelogin1Manager->CanPowerOff();
    if (reply.isError())
    {
        // 如果获取失败，就假设其可以关机，由关机操作调用时做检查
        KLOG_WARNING(LCMenu) << "failed to call CanPowerOff" << reply.error();
        return true;
    }

    QString data = reply.value();
    return (data == "yes");
}

bool Power::canReboot()
{
    RETURN_VAL_IF_TRUE(m_gsettings->get(LOCKDOWN_SCHEMA_KEY_DISABLE_REBOOT).toBool(), false);
    RETURN_VAL_IF_FALSE(m_freelogin1Manager, false);

    auto reply = m_freelogin1Manager->CanReboot();
    if (reply.isError())
    {
        // 如果获取失败，就假设其可以重启，由重启操作调用时做检查
        KLOG_WARNING(LCMenu) << "failed to call CanReboot" << reply.error();
        return true;
    }

    QString data = reply.value();
    return (data == "yes");
}

bool Power::canLogout()
{
    RETURN_VAL_IF_TRUE(m_gsettings->get(LOCKDOWN_SCHEMA_KEY_DISABLE_LOG_OUT).toBool(), false);

    return true;
}

bool Power::canSwitchUser()
{
    RETURN_VAL_IF_TRUE(m_gsettings->get(LOCKDOWN_SCHEMA_KEY_DISABLE_USER_SWITCHING).toBool(), false);
    RETURN_VAL_IF_FALSE(m_seatManagerProxy, false);

    return m_seatManagerProxy->property("CanSwitch").toBool();
}

bool Power::canLockScreen()
{
    RETURN_VAL_IF_TRUE(m_gsettings->get(LOCKDOWN_SCHEMA_KEY_DISABLE_LOCK_SCREEN).toBool(), false);

    return QFile::exists("/usr/bin/kiran-screensaver-command");
}
