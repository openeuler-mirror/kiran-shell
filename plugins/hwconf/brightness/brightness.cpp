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
#include <QDBusPendingReply>

#include "brightness.h"
#include "ks_power_interface.h"
#include "lib/common/dbus-service-watcher.h"

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
    : QObject{parent},
      m_ksPower(nullptr)
{
    QDBusConnection::sessionBus().connect(
        POWER_DBUS_SERVICE, POWER_DBUS_OBJECT_PATH, POWER_DBUS_INTERFACE,
        BRIGHTNESS_CHANGED, this,
        SLOT(dbusBrightnessChanged(QDBusMessage)));
    QDBusConnection::sessionBus().connect(
        POWER_DBUS_SERVICE, POWER_DBUS_OBJECT_PATH, POWER_DBUS_INTERFACE,
        ACTIVE_PROFILE_CHANGED, this,
        SLOT(dbusActiveProfileChanged(QDBusMessage)));

    connect(&DBusWatcher, &DBusServiceWatcher::serviceOwnerChanged, this, &Brightness::serviceOwnerChanged);
    DBusWatcher.AddService(POWER_DBUS_SERVICE, QDBusConnection::SessionBus);
}

void Brightness::serviceOwnerChanged(const QString &service, const QString &oldOwner, const QString &newOwner)
{
    if (POWER_DBUS_SERVICE != service)
    {
        return;
    }
    if (oldOwner.isEmpty())
    {
        KLOG_INFO() << "dbus service registered:" << service;
        init();
    }
    else if (newOwner.isEmpty())
    {
        KLOG_INFO() << "dbus service unregistered:" << service;
        emit enableBrightness(false);
    }
}

void Brightness::init()
{
    if (m_ksPower)
    {
        m_ksPower->deleteLater();
        m_ksPower = nullptr;
    }

    m_ksPower = new KSPower(POWER_DBUS_SERVICE,
                            POWER_DBUS_OBJECT_PATH,
                            QDBusConnection::sessionBus(),
                            this);
    if (!m_ksPower->isValid())
    {
        emit enableBrightness(false);
        return;
    }

    updateBrightness();
}

void Brightness::setBrightness(int value)
{
    if (!m_ksPower || !m_ksPower->isValid())
    {
        return;
    }

    // SetBrightness 设置亮度的接口反应很慢，需要异步调用
    auto call = m_ksPower->SetBrightness(POWER_DEVICE_TYPE_MONITOR, value);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &Brightness::handleSetBrightnessResult);
}

bool Brightness::getBrightness(int &value)
{
    if (!m_ksPower || !m_ksPower->isValid())
    {
        return false;
    }
    auto message = m_ksPower->GetBrightness(POWER_DEVICE_TYPE_MONITOR);
    if (message.isError())
    {
        KLOG_WARNING() << "GetBrightness failed:" << message;
        return false;
    }

    value = message.value();
    return true;
}

void Brightness::updateBrightness()
{
    if (!m_ksPower || !m_ksPower->isValid())
    {
        return;
    }

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
