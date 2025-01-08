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

#include <qt5-log-i.h>
#include <QDBusInterface>
#include <QIcon>

#include "ks-i.h"
#include "lib/common/dbus-service-watcher.h"
#include "theme-conf-item.h"

#define APPEARANCE_DBUS_SERVICE "com.kylinsec.Kiran.SessionDaemon.Appearance"
#define APPEARANCE_DBUS_OBJECT_PATH "/com/kylinsec/Kiran/SessionDaemon/Appearance"
#define APPEARANCE_DBUS_INTERFACE "com.kylinsec.Kiran.SessionDaemon.Appearance"

#define DARK_THEME "Kiran-dark"
#define LIGHT_THEME "Kiran-white"
#define THEME_AUTO_NAME "Kiran-auto"

namespace Kiran
{
namespace HwConf
{
ThemeConfItem::ThemeConfItem(QWidget *parent)
    : HwConfItem(parent, false)
{
    connect(this, &HwConfItem::iconClicked, this, &ThemeConfItem::themeIconClicked);

    setIcon(QIcon::fromTheme(KS_ICON_HWCONF_THEME_SWITCH));
    setTooltip(tr("theme switch"));

    connect(&DBusWatcher, &DBusServiceWatcher::serviceOwnerChanged,
            [this](const QString &service, const QString &oldOwner, const QString &newOwner)
            {
                if (APPEARANCE_DBUS_SERVICE != service)
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
                    emit enableTheme(false);
                }
            });
    DBusWatcher.AddService(APPEARANCE_DBUS_SERVICE, QDBusConnection::SessionBus);
}

void ThemeConfItem::init()
{
    m_interface = new QDBusInterface(APPEARANCE_DBUS_SERVICE,
                                     APPEARANCE_DBUS_OBJECT_PATH,
                                     APPEARANCE_DBUS_INTERFACE,
                                     QDBusConnection::sessionBus(),
                                     this);
    if (!m_interface->isValid())
    {
        emit enableTheme(false);
        return;
    }

    emit enableTheme(true);

    getTheme(m_themeID);
}

void ThemeConfItem::themeIconClicked()
{
    if (DARK_THEME == m_themeID)
    {
        m_themeID = LIGHT_THEME;
        setTheme(m_themeID);
    }
    else
    {
        m_themeID = DARK_THEME;
        setTheme(m_themeID);
    }
}

bool ThemeConfItem::getTheme(QString &themeID)
{
    if (!m_interface || !m_interface->isValid())
    {
        return false;
    }

    auto message = m_interface->call("GetTheme", APPEARANCE_THEME_TYPE_GTK);
    if (QDBusMessage::ErrorMessage == message.type() || message.arguments().size() < 1)
    {
        KLOG_WARNING() << "getTheme failed:" << message;
        return false;
    }

    themeID = message.arguments().first().toString();

    return true;
}

bool ThemeConfItem::setTheme(const QString &themeID)
{
    if (!m_interface || !m_interface->isValid())
    {
        return false;
    }

    auto message = m_interface->call("SetTheme", APPEARANCE_THEME_TYPE_GTK, themeID);
    if (QDBusMessage::ErrorMessage == message.type() || message.arguments().size() < 1)
    {
        KLOG_WARNING() << "setTheme,type:" << APPEARANCE_THEME_TYPE_GTK << "failed:" << message;
        return false;
    }

    message = m_interface->call("SetTheme", APPEARANCE_THEME_TYPE_METACITY, themeID);
    if (QDBusMessage::ErrorMessage == message.type() || message.arguments().size() < 1)
    {
        KLOG_WARNING() << "setTheme,type:" << APPEARANCE_THEME_TYPE_METACITY << "failed:" << message;
        return false;
    }

    return true;
}
}  // namespace HwConf
}  // namespace Kiran
