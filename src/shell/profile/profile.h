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

#pragma once

#include <QMap>
#include "profile-applet.h"
#include "profile-panel.h"

class QGSettings;

namespace Kiran
{
class LayoutPanel;
class LayoutApplet;

class Profile : public QObject
{
    Q_OBJECT

    GSETTINGS_PROPERTY_STRING_DECLARATION(defaultLayout, DefaultLayout)
    GSETTINGS_PROPERTY_STRINGLIST_DECLARATION(panelUIDs, PanelUIDs)
    GSETTINGS_PROPERTY_STRINGLIST_DECLARATION(appletUIDs, AppletUIDs)

public:
    static Profile* getInstance()
    {
        return m_instance;
    };

    static void globalInit();
    static void globalDeinit();

public:
    QList<ProfilePanel*> getPanels();
    QList<ProfileApplet*> getApplets();
    QList<ProfileApplet*> getAppletsOnPanel(const QString& panelUID);

private:
    Profile();

    void init();
    void initSettings();
    // 从默认布局中加载信息
    void loadFromLayout();
    void loadPanelFromLayout(LayoutPanel* layoutPanel);
    void loadAppletFromLayout(LayoutApplet* layoutApplet);
    // 从gsettings中加载信息
    void loadFromSettings();

private Q_SLOTS:
    void updateSettings(const QString& key);

private:
    static Profile* m_instance;
    // 全局配置
    QGSettings* m_settings;
    // <面板UID， 面板配置>
    QMap<QString, ProfilePanel*> m_panels;
    // <AppletUID， Applet配置>
    QMap<QString, ProfileApplet*> m_applets;
};
}  // namespace Kiran
