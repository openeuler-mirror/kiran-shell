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
    m_settings = new QGSettings(SHELL_SCHEMA_ID, "", this);

    connect(m_settings, SIGNAL(changed(const QString&)), this, SLOT(updateSettings(const QString&)));
}

void Profile::init()
{
    initSettings();

    auto panelUIDs = getPanelUIDs();
    // 如果面板数为0，则需要使用布局中的面板配置

    if (panelUIDs.empty())
    {
        loadFromLayout();
    }
    else
    {
        loadFromSettings();
    }
}

void Profile::initSettings()
{
    // 初始化阶段不发送信号，避免其他模块重复处理
    blockSignals(true);
    updateSettings(SHELL_SCHEMA_KEY_DEFAULT_LAYOUT);
    updateSettings(SHELL_SCHEMA_KEY_PANEL_UIDS);
    updateSettings(SHELL_SCHEMA_KEY_APPLET_UIDS);
    blockSignals(false);
}

void Profile::loadFromLayout()
{
    QScopedPointer<Layout> layout(new Layout(getDefaultLayout()));

    KLOG_DEBUG(LCShell) << "Load from layout " << getDefaultLayout();

    auto layoutPanels = layout->getPanels();
    for (auto* layoutPanel : layoutPanels)
    {
        loadPanelFromLayout(layoutPanel);
    }

    auto layoutApplets = layout->getApplets();
    for (auto* layoutApplet : layoutApplets)
    {
        loadAppletFromLayout(layoutApplet);
    }
}

void Profile::loadPanelFromLayout(LayoutPanel* layoutPanel)
{
    auto* panel = new ProfilePanel(layoutPanel->getUID());
    // TODO: 将layoutPanel中的属性设置到panel
    // TODO： size需要限制一个最小值
    panel->setSize(layoutPanel->getSize());
    panel->setOrientation(layoutPanel->getOrientation());
    panel->setMonitor(layoutPanel->getMonitor());
    m_panels.insert(panel->getUID(), panel);

    // 更新panel-uids属性
    auto panelUIDs = getPanelUIDs();
    if (!panelUIDs.contains(panel->getUID()))
    {
        panelUIDs.push_back(panel->getUID());
        setPanelUIDs(panelUIDs);
    }
}

void Profile::loadAppletFromLayout(LayoutApplet* layoutApplet)
{
    auto* applet = new ProfileApplet(layoutApplet->getUID());
    // TODO: 将layoutApplet中的属性设置到applet
    applet->setID(layoutApplet->getID());
    applet->setPanel(layoutApplet->getPanel());
    applet->setPosition(layoutApplet->getPosition());
    applet->setPanelRightStick(layoutApplet->getPanelRightStick());
    m_applets.insert(applet->getUID(), applet);

    auto appletUIDs = getAppletUIDs();
    if (!appletUIDs.contains(applet->getUID()))
    {
        appletUIDs.push_back(applet->getUID());
        setAppletUIDs(appletUIDs);
    }
}

void Profile::loadFromSettings()
{
    auto panelUIDs = getPanelUIDs();
    KLOG_INFO(LCShell) << "load panel:" << panelUIDs;

    for (auto& panelUID : panelUIDs)
    {
        if (!m_panels.contains(panelUID))
        {
            auto* panel = new ProfilePanel(panelUID);
            m_panels.insert(panel->getUID(), panel);
        }
    }

    auto appletUIDs = getAppletUIDs();
    KLOG_INFO(LCShell) << "load applets:" << appletUIDs;

    for (auto& appletUID : appletUIDs)
    {
        if (!m_applets.contains(appletUID))
        {
            auto* applet = new ProfileApplet(appletUID);
            m_applets.insert(applet->getUID(), applet);
        }
    }
    // 清理不存在的插件
    for (auto appletUID : m_applets.keys())
    {
        if (!appletUIDs.contains(appletUID))
        {
            delete m_applets[appletUID];
            m_applets.remove(appletUID);
        }
    }
    // 清理不存在的面板
    for (auto panelUID : m_panels.keys())
    {
        if (!panelUIDs.contains(panelUID))
        {
            delete m_panels[panelUID];
            m_panels.remove(panelUID);
        }
    }
}

QList<ProfilePanel*> Profile::getPanels()
{
    return m_panels.values();
}

QList<ProfileApplet*> Profile::getApplets()
{
    return m_applets.values();
}

QList<ProfileApplet*> Profile::getAppletsOnPanel(const QString& panelUID)
{
    loadFromSettings();

    QList<ProfileApplet*> applets;
    QList<ProfileApplet*> applets_right;

    for (auto& applet : m_applets)
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
