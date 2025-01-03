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

#include <kiran-cc-daemon/kiran-session-daemon/power-i.h>
#include <qt5-log-i.h>
#include <QDBusInterface>
#include <QDBusPendingReply>

#include "brightness.h"

#define POWER_DBUS_SERVICE "com.kylinsec.Kiran.SessionDaemon.Power"
#define POWER_DBUS_OBJECT_PATH "/com/kylinsec/Kiran/SessionDaemon/Power"
#define POWER_DBUS_INTERFACE "com.kylinsec.Kiran.SessionDaemon.Power"
#define BRIGHTNESS_CHANGED "BrightnessChanged"
#define ACTIVE_PROFILE_CHANGED "ActiveProfileChanged"

namespace Kiran
{
namespace HwConf
{
Brightness::Brightness(QObject *parent)
    : QObject{parent}
{
}

void Brightness::init()
{
    m_interface = new QDBusInterface(POWER_DBUS_SERVICE,
                                     POWER_DBUS_OBJECT_PATH,
                                     POWER_DBUS_INTERFACE,
                                     QDBusConnection::sessionBus(),
                                     this);

    QDBusConnection::sessionBus().connect(
        POWER_DBUS_SERVICE, POWER_DBUS_OBJECT_PATH, POWER_DBUS_INTERFACE,
        BRIGHTNESS_CHANGED, this,
        SLOT(dbusBrightnessChanged(QDBusMessage)));
    QDBusConnection::sessionBus().connect(
        POWER_DBUS_SERVICE, POWER_DBUS_OBJECT_PATH, POWER_DBUS_INTERFACE,
        ACTIVE_PROFILE_CHANGED, this,
        SLOT(dbusActiveProfileChanged(QDBusMessage)));

    updateBrightness();
}

void Brightness::setBrightness(int value)
{
    // SetBrightness 设置亮度的接口反应很慢，需要异步调用
    auto call = m_interface->asyncCall("SetBrightness", POWER_DEVICE_TYPE_MONITOR, value);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &Brightness::handleSetBrightnessResult);
}

bool Brightness::getBrightness(int &value)
{
    auto message = m_interface->call("GetBrightness", POWER_DEVICE_TYPE_MONITOR);
    if (QDBusMessage::ErrorMessage == message.type() || message.arguments().size() < 1)
    {
        KLOG_WARNING() << "GetBrightness failed:" << message;
        return false;
    }

    value = message.arguments().first().toInt();
    return true;
}

void Brightness::updateBrightness()
{
    bool checkReadyToUpdate = false;
    emit isReadyToUpdate(checkReadyToUpdate);
    if (!checkReadyToUpdate)
    {
        return;
    }

    int brightnessValue;
    if (!getBrightness(brightnessValue))
    {
        return;
    }
    auto message = m_interface->call("GetBrightness", POWER_DEVICE_TYPE_MONITOR);
    if (QDBusMessage::ErrorMessage == message.type() || message.arguments().size() < 1)
    {
        KLOG_WARNING() << "GetBrightness failed:" << message;
        return;
    }

    // 亮度为-1表示亮度调整不可用
    if (-1 == brightnessValue)
    {
        emit enableBrightness(false);
    }
    else
    {
        emit brightnessValueChanged(brightnessValue);
    }

    KLOG_INFO() << "updateBrightness" << brightnessValue;
}

void Brightness::dbusBrightnessChanged(QDBusMessage message)
{
    KLOG_INFO() << "dbusBrightnessChanged" << message;
    updateBrightness();
}

void Brightness::dbusActiveProfileChanged(QDBusMessage message)
{
    KLOG_INFO() << "dbusActiveProfileChanged" << message;
    updateBrightness();
}

void Brightness::handleSetBrightnessResult(QDBusPendingCallWatcher *watcher)
{
    if (!watcher)
    {
        return;
    }

    QDBusPendingReply<> reply = *watcher;
    if (reply.isError())
    {
        KLOG_WARNING() << "SetBrightness failed:" << reply.error().message();
    }

    watcher->deleteLater();
}
}  // namespace HwConf
}  // namespace Kiran
