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

#include <qt5-log-i.h>
#include <QHBoxLayout>

#include "applet.h"
#include "lib/common/logging-category.h"
#include "plugin.h"
#include "profile/profile-applet.h"
#include "shell.h"

namespace Kiran
{
AppletImport::AppletImport(Applet *applet, QObject *parent)
    : QObject(parent),
      m_applet(applet)
{
}

IPanel *AppletImport::getPanel()
{
    return this->m_applet->getPanel();
}

IApplet *AppletImport::getApplet()
{
    return this->m_applet;
}

Applet::Applet(ProfileApplet *profileApplet,
               Panel *panel)
    : QWidget(panel),
      m_profileApplet(profileApplet),
      m_panel(panel),
      m_appletImport(nullptr),
      m_pluginApplet(nullptr)
{
    this->m_appletImport = new AppletImport(this, this);
    this->init();
}

Panel *Applet::getPanel()
{
    return this->m_panel;
}

QString Applet::getID()
{
    return m_profileApplet->getID();
}

void Applet::init()
{
    auto appletID = this->m_profileApplet->getID();
    auto plugin = PluginPool::getInstance()->findPluginForApplet(appletID);
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    if (!plugin)
    {
        KLOG_WARNING(LCShell) << "Not found plugin for applet id: " << appletID;
        return;
    }

    KLOG_DEBUG(LCShell) << "Found plugin for appletID " << appletID;
    this->m_pluginApplet = plugin->createApplet(appletID, this->m_appletImport);
    if (!this->m_pluginApplet)
    {
        KLOG_WARNING(LCShell) << "Create applet " << appletID << " failed.";
        return;
    }

    layout->addWidget(m_pluginApplet);
}

}  // namespace Kiran
