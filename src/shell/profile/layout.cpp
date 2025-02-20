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
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#include <qgsettings.h>
#include <qt5-log-i.h>
#include <QDir>
#include <QFile>
#include <QGlobalStatic>
#include <QScopedPointer>
#include <QSettings>
#include <QVariant>

#include "ks-config.h"
#include "layout.h"
#include "lib/common/logging-category.h"

namespace Kiran
{
#define KS_PANEL_GROUP_NAME_PREFIX "Panel "
#define KS_PANEL_KEYFILE_KEY_SIZE "size"
#define KS_PANEL_KEYFILE_KEY_ORIENTATION "orientation"
#define KS_PANEL_KEYFILE_KEY_MONITOR "monitor"

#define KS_APPLET_GROUP_NAME_PREFIX "Applet "
#define KS_APPLET_KEYFILE_KEY_TYPE "type"
#define KS_APPLET_KEYFILE_KEY_ID "id"
#define KS_APPLET_KEYFILE_KEY_PANEL "panel"
#define KS_APPLET_KEYFILE_KEY_POSITION "position"
#define KS_APPLET_KEYFILE_KEY_PRS "panel-right-stick"

#define LAYOUT_PROPERTY_INT_DEFINITION(className, name, humpName, key) \
    int className::get##humpName()                                     \
    {                                                                  \
        return value(key).toInt();                                     \
    }                                                                  \
    void className::set##humpName(int value)                           \
    {                                                                  \
        if (value != get##humpName())                                  \
        {                                                              \
            setValue(key, value);                                      \
        }                                                              \
    }

#define LAYOUT_PROPERTY_BOOLEAN_DEFINITION(className, name, humpName, key) \
    bool className::get##humpName()                                        \
    {                                                                      \
        return value(key).toBool();                                        \
    }                                                                      \
    void className::set##humpName(bool value)                              \
    {                                                                      \
        if (value != get##humpName())                                      \
        {                                                                  \
            setValue(key, value);                                          \
        }                                                                  \
    }

#define LAYOUT_PROPERTY_STRING_DEFINITION(className, name, humpName, key) \
    QString className::get##humpName()                                    \
    {                                                                     \
        return value(key).toString();                                     \
    }                                                                     \
    void className::set##humpName(const QString &value)                   \
    {                                                                     \
        if (value != get##humpName())                                     \
        {                                                                 \
            setValue(key, value);                                         \
        }                                                                 \
    }

Group::Group(const QString &groupName,
             QSettings *settings,
             QObject *parent)
    : QObject(parent),
      m_groupName(groupName),
      m_settings(settings)
{
}

void Group::setValue(const QString &key, const QVariant &value)
{
    return m_settings->setValue(QString("%1/%2").arg(m_groupName, key), value);
}

QVariant Group::value(const QString &key, const QVariant &defaultValue) const
{
    return m_settings->value(QString("%1/%2").arg(m_groupName, key), defaultValue);
}

LAYOUT_PROPERTY_INT_DEFINITION(LayoutPanel, size, Size, KS_PANEL_KEYFILE_KEY_SIZE)
LAYOUT_PROPERTY_STRING_DEFINITION(LayoutPanel, orientation, Orientation, KS_PANEL_KEYFILE_KEY_ORIENTATION)
LAYOUT_PROPERTY_INT_DEFINITION(LayoutPanel, monitor, Monitor, KS_PANEL_KEYFILE_KEY_MONITOR)

LayoutPanel::LayoutPanel(const QString &panelUID,
                         QSettings *settings,
                         QObject *parent)
    : Group(KS_PANEL_GROUP_NAME_PREFIX + panelUID, settings, parent),
      m_panelUID(panelUID)
{
}

LAYOUT_PROPERTY_STRING_DEFINITION(LayoutApplet, type, Type, KS_APPLET_KEYFILE_KEY_TYPE)
LAYOUT_PROPERTY_STRING_DEFINITION(LayoutApplet, id, ID, KS_APPLET_KEYFILE_KEY_ID)
LAYOUT_PROPERTY_STRING_DEFINITION(LayoutApplet, panel, Panel, KS_APPLET_KEYFILE_KEY_PANEL)
LAYOUT_PROPERTY_INT_DEFINITION(LayoutApplet, position, Position, KS_APPLET_KEYFILE_KEY_POSITION)
LAYOUT_PROPERTY_BOOLEAN_DEFINITION(LayoutApplet, panelRightStick, PanelRightStick, KS_APPLET_KEYFILE_KEY_PRS)

LayoutApplet::LayoutApplet(const QString &appletUID,
                           QSettings *settings,
                           QObject *parent)
    : Group(KS_APPLET_GROUP_NAME_PREFIX + appletUID, settings, parent),
      m_appletUID(appletUID)
{
}

Layout::Layout(const QString &layoutName)
    : m_settings(nullptr)
{
    m_layoutFilePath = QString("%1/%2.layout").arg(KS_LAYOUTDIR).arg(layoutName);
    if (!QFile::exists(m_layoutFilePath))
    {
        KLOG_WARNING(LCShell) << "Not found the layout file " << m_layoutFilePath;
    }

    load();
}

QList<LayoutPanel *> Layout::getPanels()
{
    return m_panels.values();
}

QList<LayoutApplet *> Layout::getApplets()
{
    return m_applets.values();
}

QList<LayoutApplet *> Layout::getAppletsOnPanel(const QString &panelUID)
{
    QList<LayoutApplet *> applets;

    for (auto &applet : m_applets)
    {
        if (applet->getPanel() == panelUID)
        {
            applets.push_back(applet);
        }
    }
    return applets;
}

void Layout::load()
{
    m_settings = new QSettings(m_layoutFilePath, QSettings::Format::IniFormat, this);

    for (auto &groupName : m_settings->childGroups())
    {
        if (groupName.startsWith(KS_PANEL_GROUP_NAME_PREFIX))
        {
            auto panelUID = groupName.mid(QStringLiteral(KS_PANEL_GROUP_NAME_PREFIX).length());
            auto panel = new LayoutPanel(panelUID, m_settings, this);
            m_panels.insert(panelUID, panel);
        }
        else if (groupName.startsWith(KS_APPLET_GROUP_NAME_PREFIX))
        {
            auto appletUID = groupName.mid(QStringLiteral(KS_APPLET_GROUP_NAME_PREFIX).length());
            auto applet = new LayoutApplet(appletUID, m_settings, this);
            m_applets.insert(appletUID, applet);
        }
        else
        {
            KLOG_WARNING(LCShell) << "Unknown group name: " << groupName;
        }
    }
}

}  // namespace Kiran
