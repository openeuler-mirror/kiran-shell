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
#include <QGSettings>
#include <QScopedPointer>

#include "ks-definition.h"
#include "ks-i.h"
#include "layout.h"
#include "lib/common/logging-category.h"
#include "profile-applet.h"
#include "profile-panel.h"
#include "profile.h"
namespace Kiran
{
GSETTINGS_PROPERTY_STRING_DEFINITION(Profile, defaultLayout, DefaultLayout, SHELL_SCHEMA_KEY_DEFAULT_LAYOUT)
GSETTINGS_PROPERTY_STRINGLIST_DEFINITION(Profile, panelUIDs, PanelUIDs, SHELL_SCHEMA_KEY_PANEL_UIDS)
GSETTINGS_PROPERTY_STRINGLIST_DEFINITION(Profile, appletUIDs, AppletUIDs, SHELL_SCHEMA_KEY_APPLET_UIDS)

Profile* Profile::m_instance = nullptr;
void Profile::globalInit()
{
    m_instance = new Profile();
    m_instance->init();
}

void Profile::globalDeinit()
{
    delete m_instance;
}

Profile::Profile()
{
    this->m_settings = new QGSettings(SHELL_SCHEMA_ID, "", this);

    connect(this->m_settings, SIGNAL(changed(const QString&)), this, SLOT(updateSettings(const QString&)));
}

void Profile::init()
{
    this->initSettings();

    auto panelUIDs = this->getPanelUIDs();
    // 如果面板数为0，则需要使用布局中的面板配置

    if (panelUIDs.size() == 0)
    {
        this->loadFromLayout();
    }
    else
    {
        this->loadFromSettings();
    }
}

void Profile::initSettings()
{
    // 初始化阶段不发送信号，避免其他模块重复处理
    this->blockSignals(true);
    this->updateSettings(SHELL_SCHEMA_KEY_DEFAULT_LAYOUT);
    this->updateSettings(SHELL_SCHEMA_KEY_PANEL_UIDS);
    this->updateSettings(SHELL_SCHEMA_KEY_APPLET_UIDS);
    this->blockSignals(false);
}

void Profile::loadFromLayout()
{
    QScopedPointer<Layout> layout(new Layout(this->getDefaultLayout()));

    KLOG_DEBUG(LCShell) << "Load from layout " << this->getDefaultLayout();

    auto layoutPanels = layout->getPanels();
    for (auto layoutPanel : layoutPanels)
    {
        this->loadPanelFromLayout(layoutPanel);
    }

    auto layoutApplets = layout->getApplets();
    for (auto layoutApplet : layoutApplets)
    {
        this->loadAppletFromLayout(layoutApplet);
    }
}

void Profile::loadPanelFromLayout(LayoutPanel* layoutPanel)
{
    auto panel = new ProfilePanel(layoutPanel->getUID());
    // TODO: 将layoutPanel中的属性设置到panel
    // TODO： size需要限制一个最小值
    panel->setSize(layoutPanel->getSize());
    panel->setOrientation(layoutPanel->getOrientation());
    panel->setMonitor(layoutPanel->getMonitor());
    this->m_panels.insert(panel->getUID(), panel);

    // 更新panel-uids属性
    auto panelUIDs = this->getPanelUIDs();
    if (!panelUIDs.contains(panel->getUID()))
    {
        panelUIDs.push_back(panel->getUID());
        this->setPanelUIDs(panelUIDs);
    }
}

void Profile::loadAppletFromLayout(LayoutApplet* layoutApplet)
{
    auto applet = new ProfileApplet(layoutApplet->getUID());
    // TODO: 将layoutApplet中的属性设置到applet
    applet->setID(layoutApplet->getID());
    applet->setPanel(layoutApplet->getPanel());
    applet->setPosition(layoutApplet->getPosition());
    applet->setPanelRightStick(layoutApplet->getPanelRightStick());
    this->m_applets.insert(applet->getUID(), applet);

    auto appletUIDs = this->getAppletUIDs();
    if (!appletUIDs.contains(applet->getUID()))
    {
        appletUIDs.push_back(applet->getUID());
        this->setAppletUIDs(appletUIDs);
    }
}

void Profile::loadFromSettings()
{
    auto panelUIDs = this->getPanelUIDs();
    for (auto& panelUID : panelUIDs)
    {
        auto panel = new ProfilePanel(panelUID);
        this->m_panels.insert(panel->getUID(), panel);
    }

    auto appletUIDs = this->getAppletUIDs();
    for (auto& appletUID : appletUIDs)
    {
        auto applet = new ProfileApplet(appletUID);
        this->m_applets.insert(applet->getUID(), applet);
    }
}

QList<ProfilePanel*> Profile::getPanels()
{
    return this->m_panels.values();
}

QList<ProfileApplet*> Profile::getApplets()
{
    return this->m_applets.values();
}

QList<ProfileApplet*> Profile::getAppletsOnPanel(const QString& panelUID)
{
    QList<ProfileApplet*> applets;
    QList<ProfileApplet*> applets_right;

    for (auto& applet : this->m_applets)
    {
        if (applet->getPanel() == panelUID)
        {
            if (applet->getPanelRightStick())
            {
                applets_right.push_back(applet);
            }
            else
            {
                applets.push_back(applet);
            }
        }
    }

    // 各插件按gsettings中的顺序排列
    std::sort(applets.begin(), applets.end(), [](ProfileApplet* a, ProfileApplet* b)
              {
                  return a->getPosition() < b->getPosition();
              });

    std::sort(applets_right.begin(), applets_right.end(), [](ProfileApplet* a, ProfileApplet* b)
              {
                  return a->getPosition() > b->getPosition();
              });
    applets.append(applets_right);

    return applets;
}

void Profile::updateSettings(const QString& key)
{
    switch (shash(key.toUtf8().data()))
    {
        GSETTINGS_CASE_STRING_CHANGE(SHELL_SCHEMA_KEY_DEFAULT_LAYOUT, DefaultLayout)
        GSETTINGS_CASE_STRINGLIST_CHANGE(SHELL_SCHEMA_KEY_PANEL_UIDS, PanelUIDs)
        GSETTINGS_CASE_STRINGLIST_CHANGE(SHELL_SCHEMA_KEY_APPLET_UIDS, AppletUIDs)
        GSETTINGS_CASE_DEFAULT
    }
}
}  // namespace Kiran
