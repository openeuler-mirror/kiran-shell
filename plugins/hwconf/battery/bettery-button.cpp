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

#include <libupower-glib/upower.h>
#include <qt5-log-i.h>
#include <QDBusArgument>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QGSettings>

#include "bettery-button.h"
#include "lib/common/dbus-service-watcher.h"

#define UPOWER_DBUS_SERVICE "org.freedesktop.UPower"
#define UPOWER_DBUS_OBJECT_PATH "/org/freedesktop/UPower"
#define UPOWER_DBUS_INTERFACE "org.freedesktop.UPower"
#define UPOWER_DEVICE_DBUS_INTERFACE "org.freedesktop.UPower.Device"
#define UPOWER_DISPLAY_DEVICE_DBUS_SERVICE "org.freedesktop.UPower.devices.DisplayDevice"
#define UPOWER_DISPLAY_DEVICE_DBUS_OBJECT_PATH "/org/freedesktop/UPower/devices/DisplayDevice"
#define UPOWER_DBUS_PROP_ON_BATTERY "OnBattery"

#define PROPERTIES_INTERFACE "org.freedesktop.DBus.Properties"
#define PROPERTIES_CHANGED "PropertiesChanged"

#define POWER_SCHEMA_ID "com.kylinsec.kiran.power"
#define POWER_SCHEMA_TRAY_ICON_POLICY "trayIconPolicy"

#define DEFAULT_ICON_NAME "ksvg-ks-ac-adapter-symbolic"

namespace Kiran
{
namespace HwConf
{
BatteryButton::BatteryButton(QWidget *parent)
    : HwConfButton(parent),
      m_interface(nullptr),
      m_interfaceDevice(nullptr)
{
    QDBusConnection::systemBus().connect(
        UPOWER_DBUS_SERVICE, "", PROPERTIES_INTERFACE,
        PROPERTIES_CHANGED, this,
        SLOT(uPowerdDusPropertiesChanged(QDBusMessage)));

    connect(&DBusWatcher, &DBusServiceWatcher::serviceOwnerChanged,
            [this](const QString &service, const QString &oldOwner, const QString &newOwner)
            {
                if (UPOWER_DBUS_SERVICE != service)
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
                    disableBattery();
                }
            });
    DBusWatcher.AddService(UPOWER_DBUS_SERVICE, QDBusConnection::SessionBus);

    m_gsettings = new QGSettings(POWER_SCHEMA_ID);
    connect(m_gsettings, &QGSettings::changed, this, &BatteryButton::settingChanged);
}

void BatteryButton::init()
{
    if (m_interface)
    {
        m_interface->deleteLater();
        m_interface = nullptr;
    }
    m_interface = new QDBusInterface(UPOWER_DBUS_SERVICE, UPOWER_DBUS_OBJECT_PATH, UPOWER_DBUS_INTERFACE, QDBusConnection::systemBus(), this);
    if (!m_interface->isValid())
    {
        disableBattery();
        return;
    }

    updateDisplayDevice();
    updateIcon();
}

void BatteryButton::settingChanged(const QString &key)
{
    if (POWER_SCHEMA_TRAY_ICON_POLICY == key)
    {
        updateIcon();
    }
}

void BatteryButton::updateIcon()
{
    QString iconName = getIconName();
    auto iconPolicy = m_gsettings->get(POWER_SCHEMA_TRAY_ICON_POLICY).toString();
    if ("always" == iconPolicy)  // 永久显示
    {
        // 没有获取到图标，则使用默认图标
        if (iconName.isEmpty())
        {
            iconName = DEFAULT_ICON_NAME;
        }
    }
    else if ("never" == iconPolicy)  // 永不显示
    {
        iconName.clear();
    }

    if (iconName.isEmpty())
    {
        disableBattery();
        return;
    }

    QIcon icon = QIcon::fromTheme(iconName);
    setIcon(icon);
    setVisible(true);
    emit enableBattery(true);
    emit batteryIconChanged(icon);
}

QString BatteryButton::getIconName()
{
    if (!m_interfaceDevice)
    {
        return "";
    }

    double percentage = -1;
    QString iconName;

    auto deviceType = m_interfaceDevice->property("Type");
    if (!deviceType.isValid() || (UP_DEVICE_KIND_BATTERY != deviceType.toUInt() && UP_DEVICE_KIND_UPS != deviceType.toUInt()))
    {
        emit batteryValueChanged("");
        return iconName;
    }

    // 电池和UPS有电量概念
    auto deviceState = m_interfaceDevice->property("State");
    auto valuePercentage = m_interfaceDevice->property("Percentage");
    if (deviceState.isValid() && valuePercentage.isValid())
    {
        auto state = deviceState.toUInt();
        percentage = valuePercentage.toDouble();
        QString iconIndex = percent2IconIndex(percentage);

        switch (state)
        {
        case UP_DEVICE_STATE_EMPTY:
            // 电量耗尽，红色图标
            iconName = "ks-battery-000";
            break;
        case UP_DEVICE_STATE_FULLY_CHARGED:
        case UP_DEVICE_STATE_CHARGING:
        case UP_DEVICE_STATE_PENDING_CHARGE:
            // 充电中
            iconName = QString("ksvg-ks-battery-%1-charging-symbolic").arg(iconIndex);
            break;
        case UP_DEVICE_STATE_DISCHARGING:
        case UP_DEVICE_STATE_PENDING_DISCHARGE:
            // 未充电
            if (iconIndex == "000")
            {
                iconName = "ks-battery-000";
            }
            else
            {
                iconName = QString("ksvg-ks-battery-%1-symbolic").arg(iconIndex);
            }
            break;
        default:
            iconName = DEFAULT_ICON_NAME;
            break;
        }
    }

    if (-1 == percentage)
    {
        emit batteryValueChanged("");
    }
    else
    {
        emit batteryValueChanged(QString::number(percentage));
    }
    return iconName;
}

void BatteryButton::updateDisplayDevice()
{
    if (!m_interface || !m_interface->isValid())
    {
        return;
    }

    auto message = m_interface->call("GetDisplayDevice");
    if (QDBusMessage::ErrorMessage == message.type() || message.arguments().size() < 1)
    {
        KLOG_ERROR() << "GetDisplayDevice failed:" << message;
        return;
    }

    auto displayDevicePath = message.arguments().first().value<QDBusObjectPath>().path();
    if (m_interfaceDevice)
    {
        m_interfaceDevice->deleteLater();
        m_interfaceDevice = nullptr;
    }

    m_interfaceDevice = new QDBusInterface(UPOWER_DBUS_SERVICE, displayDevicePath, UPOWER_DEVICE_DBUS_INTERFACE, QDBusConnection::systemBus(), this);
}

void BatteryButton::disableBattery()
{
    setVisible(false);
    emit enableBattery(false);
}

QString BatteryButton::percent2IconIndex(uint percentage)
{
    if (percentage <= 10)
    {
        return "000";
    }
    else if (percentage <= 30)
    {
        return "020";
    }
    else if (percentage <= 50)
    {
        return "040";
    }
    else if (percentage <= 70)
    {
        return "060";
    }
    else if (percentage <= 90)
    {
        return "080";
    }

    return "100";
}

void BatteryButton::uPowerdDusPropertiesChanged(QDBusMessage message)
{
    const QList<QVariant> &args = message.arguments();
    if (args.size() < 2)
    {
        qWarning() << "Invalid PropertiesChanged signal";
        return;
    }

    QVariant changedProperties = args.at(1);
    const QDBusArgument &dbusArg = changedProperties.value<QDBusArgument>();
    QMap<QString, QVariant> properties;
    dbusArg >> properties;
    // DisplayDevice 变化
    if (properties.contains("DisplayDevice"))
    {
        KLOG_INFO() << UPOWER_DBUS_SERVICE << "DisplayDevice changed";
        updateDisplayDevice();
    }

    // 电量变化
    // 充电变化
    updateIcon();
}
}  // namespace HwConf
}  // namespace Kiran
