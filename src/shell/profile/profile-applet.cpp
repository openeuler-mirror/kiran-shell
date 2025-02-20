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

#include <QGSettings>
#include <QVariant>

#include "ks-definition.h"
#include "ks-i.h"
#include "profile-applet.h"

namespace Kiran
{
GSETTINGS_PROPERTY_STRING_DEFINITION(ProfileApplet, id, ID, APPLET_SCHEMA_KEY_ID);
GSETTINGS_PROPERTY_STRING_DEFINITION(ProfileApplet, panel, Panel, APPLET_SCHEMA_KEY_PANEL);
GSETTINGS_PROPERTY_INT_DEFINITION(ProfileApplet, position, Position, APPLET_SCHEMA_KEY_POSITION)
GSETTINGS_PROPERTY_BOOLEAN_DEFINITION(ProfileApplet, panelRightStick, PanelRightStick, APPLET_SCHEMA_KEY_PRS)

ProfileApplet::ProfileApplet(QString uid)
    : m_position(0),
      m_panelRightStick(false),
      m_uid(std::move(uid))
{
    auto schemaPath = QString("%1/%2/").arg(APPLET_SCHEMA_PATH).arg(m_uid);
    m_settings = new QGSettings(APPLET_SCHEMA_ID, schemaPath.toUtf8(), this);

    init();

    connect(m_settings, SIGNAL(changed(const QString &)), this, SLOT(updateSettings(const QString &)));
}

void ProfileApplet::init()
{
    initSettings();
}

void ProfileApplet::initSettings()
{
    // 初始化阶段不发送信号，避免其他模块重复处理
    blockSignals(true);
    updateSettings(APPLET_SCHEMA_KEY_ID);
    updateSettings(APPLET_SCHEMA_KEY_PANEL);
    updateSettings(APPLET_SCHEMA_KEY_POSITION);
    updateSettings(APPLET_SCHEMA_KEY_PRS);
    blockSignals(false);
}

void ProfileApplet::updateSettings(const QString &key)
{
    switch (shash(key.toUtf8().data()))
    {
        GSETTINGS_CASE_STRING_CHANGE(APPLET_SCHEMA_KEY_ID, ID)
        GSETTINGS_CASE_STRING_CHANGE(APPLET_SCHEMA_KEY_PANEL, Panel)
        GSETTINGS_CASE_INT_CHANGE(APPLET_SCHEMA_KEY_POSITION, Position)
        GSETTINGS_CASE_INT_CHANGE(APPLET_SCHEMA_KEY_PRS, PanelRightStick)
        GSETTINGS_CASE_DEFAULT
    }
}
}  // namespace Kiran
