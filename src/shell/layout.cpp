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
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#include "src/shell/layout.h"
#include <qgsettings.h>
#include <qt5-log-i.h>
#include <QDir>
#include <QFile>
#include <QGlobalStatic>
#include <QScopedPointer>
#include <QSettings>
#include "ks-config.h"

namespace Kiran
{
namespace Model
{
#define PANEL_GROUP_PREFIX "Panel "
#define APPLET_GROUP_PREFIX "Applet "

Group::Group(const QString &groupName, QSettings *settings) : m_groupName(groupName),
                                                              m_settings(settings)
{
}

void Group::setValue(const QString &key, const QVariant &value)
{
    return this->m_settings->setValue(QString("%1/%2").arg(this->m_groupName, key), value);
}

QVariant Group::value(const QString &key, const QVariant &defaultValue) const
{
    return this->m_settings->value(QString("%1/%2").arg(this->m_groupName, key), defaultValue);
}

Panel::Panel(const QString &panelUID,
             QSettings *settings) : Group(PANEL_GROUP_PREFIX + panelUID, settings),
                                    m_panelUID(panelUID)
{
}

Applet::Applet(const QString &appletUID,
               QSettings *settings) : Group(APPLET_GROUP_PREFIX + appletUID, settings),
                                      m_appletUID(appletUID)
{
}

Q_GLOBAL_STATIC(Layout, gs_defaultLayout)

Layout *Layout::getInstance()
{
    return gs_defaultLayout;
}

Layout::Layout()
{
    this->m_layoutFilePath = QString("%1/.config/%2/shell.layout").arg(QDir::homePath()).arg(PROJECT_NAME);
    this->updateLayoutFile();
    this->loadLayout();
}

QList<QSharedPointer<Panel>> Layout::getPanels()
{
    return this->m_panels.values();
}

QList<QSharedPointer<Applet>> Layout::getAppletsOnPanel(const QString &panelUID)
{
    QList<QSharedPointer<Applet>> applets;

    for (auto &applet : this->m_applets)
    {
        if (applet->getPanel() == panelUID)
        {
            applets.push_back(applet);
        }
    }
    return applets;
}

void Layout::loadLayout()
{
    this->m_settings = new QSettings(this->m_layoutFilePath, QSettings::Format::IniFormat, this);

    for (auto &groupName : this->m_settings->childGroups())
    {
        if (groupName.startsWith(PANEL_GROUP_PREFIX))
        {
            auto panelUID = groupName.mid(QStringLiteral(PANEL_GROUP_PREFIX).length());
            auto panel = QSharedPointer<Panel>::create(panelUID, this->m_settings);
            this->m_panels.insert(panelUID, panel);
        }
        else if (groupName.startsWith(APPLET_GROUP_PREFIX))
        {
            auto appletUID = groupName.mid(QStringLiteral(APPLET_GROUP_PREFIX).length());
            auto applet = QSharedPointer<Applet>::create(appletUID, this->m_settings);
            this->m_applets.insert(appletUID, applet);
        }
        else
        {
            KLOG_WARNING() << "Unknown group name: " << groupName;
        }
    }
}

void Layout::updateLayoutFile()
{
    QFileInfo fileInfo(this->m_layoutFilePath);
    auto dir = fileInfo.dir();
    if (!dir.exists())
    {
        dir.mkpath(dir.dirName());
    }

    // 判断用户目录下的布局文件是否存在，如果不存在则将系统默认布局文件拷贝到用户目录
    if (!QFile::exists(this->m_layoutFilePath))
    {
        this->resetLayoutFile();
    }
}

void Layout::resetLayoutFile()
{
    QScopedPointer<QGSettings> gsettings(new QGSettings(KS_SCHEMA_ID));
    auto defaultLayout = gsettings->get(KS_SCHEMA_KEY_DEFAULT_LAYOUT).toString();
    auto defaultLayoutFile = QString("%1/layouts/%2.layout").arg(KS_INSTALL_DATADIR).arg(defaultLayout);

    if (!QFile::exists(defaultLayoutFile))
    {
        KLOG_WARNING() << "Not found default layout file " << defaultLayoutFile;
        return;
    }

    QFile file(defaultLayoutFile);
    if (!file.copy(this->m_layoutFilePath))
    {
        KLOG_WARNING() << "Failed to copy layout file " << defaultLayoutFile << " to " << this->m_layoutFilePath;
    }
}

}  // namespace Model
}  // namespace Kiran
