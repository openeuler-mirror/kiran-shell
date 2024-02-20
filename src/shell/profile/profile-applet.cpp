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

#include "src/shell/profile/profile-applet.h"
#include <ks-definition.h>
#include <QGSettings>
#include <QVariant>

namespace Kiran
{
#define KS_APPLET_SCHEMA_ID "com.kylinsec.kiran.shell.applet"
#define KS_APPLET_SCHEMA_PATH "/com/kylinsec/kiran/shell/applets"
#define KS_APPLET_SCHEMA_KEY_ID "id"
#define KS_APPLET_SCHEMA_KEY_PANEL "panel"
#define KS_APPLET_SCHEMA_KEY_POSITION "position"
#define KS_APPLET_SCHEMA_KEY_PRS "panel-right-stick"

GSETTINGS_PROPERTY_STRING_DEFINITION(ProfileApplet, id, ID, KS_APPLET_SCHEMA_KEY_ID);
GSETTINGS_PROPERTY_STRING_DEFINITION(ProfileApplet, panel, Panel, KS_APPLET_SCHEMA_KEY_PANEL);
GSETTINGS_PROPERTY_INT_DEFINITION(ProfileApplet, position, Position, KS_APPLET_SCHEMA_KEY_POSITION)
GSETTINGS_PROPERTY_BOOLEAN_DEFINITION(ProfileApplet, panelRightStick, PanelRightStick, KS_APPLET_SCHEMA_KEY_PRS)

ProfileApplet::ProfileApplet(const QString &uid)
    : m_uid(uid),
      m_position(0),
      m_panelRightStick(false)
{
    auto schemaPath = QString("%1/%2/").arg(KS_APPLET_SCHEMA_PATH).arg(this->m_uid);
    this->m_settings = new QGSettings(KS_APPLET_SCHEMA_ID, schemaPath.toUtf8(), this);

    this->init();

    connect(this->m_settings, SIGNAL(changed(const QString &)), this, SLOT(updateSettings(const QString &)));
}

void ProfileApplet::init()
{
    this->initSettings();
}

void ProfileApplet::initSettings()
{
    // 初始化阶段不发送信号，避免其他模块重复处理
    this->blockSignals(true);
    this->updateSettings(KS_APPLET_SCHEMA_KEY_ID);
    this->updateSettings(KS_APPLET_SCHEMA_KEY_PANEL);
    this->updateSettings(KS_APPLET_SCHEMA_KEY_POSITION);
    this->updateSettings(KS_APPLET_SCHEMA_KEY_PRS);
    this->blockSignals(false);
}

void ProfileApplet::updateSettings(const QString &key)
{
    switch (shash(key.toUtf8().data()))
    {
        GSETTINGS_CASE_STRING_CHANGE(KS_APPLET_SCHEMA_KEY_ID, ID)
        GSETTINGS_CASE_STRING_CHANGE(KS_APPLET_SCHEMA_KEY_PANEL, Panel)
        GSETTINGS_CASE_INT_CHANGE(KS_APPLET_SCHEMA_KEY_POSITION, Position)
        GSETTINGS_CASE_INT_CHANGE(KS_APPLET_SCHEMA_KEY_PRS, PanelRightStick)
        GSETTINGS_CASE_DEFAULT
    }
}
}  // namespace Kiran
