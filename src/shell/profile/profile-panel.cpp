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
#include <QVariant>

#include "ks-definition.h"
#include "ks-i.h"
#include "profile-panel.h"

namespace Kiran
{
GSETTINGS_PROPERTY_INT_DEFINITION(ProfilePanel, size, Size, PANEL_SCHEMA_KEY_SIZE)
GSETTINGS_PROPERTY_STRING_DEFINITION(ProfilePanel, orientation, Orientation, PANEL_SCHEMA_KEY_ORIENTATION)
GSETTINGS_PROPERTY_INT_DEFINITION(ProfilePanel, monitor, Monitor, PANEL_SCHEMA_KEY_MONITOR)

ProfilePanel::ProfilePanel(const QString &uid)
    : m_uid(uid),
      m_size(100),
      m_monitor(-1)
{
    auto schemaPath = QString("%1/%2/").arg(PANEL_SCHEMA_PATH).arg(this->m_uid);
    this->m_settings = new QGSettings(PANEL_SCHEMA_ID, schemaPath.toUtf8(), this);

    this->init();

    connect(this->m_settings, SIGNAL(changed(const QString &)), this, SLOT(updateSettings(const QString &)));
}

void ProfilePanel::init()
{
    this->initSettings();
}

void ProfilePanel::initSettings()
{
    // 初始化阶段不发送信号，避免其他模块重复处理
    this->blockSignals(true);
    this->updateSettings(PANEL_SCHEMA_KEY_SIZE);
    this->updateSettings(PANEL_SCHEMA_KEY_ORIENTATION);
    this->updateSettings(PANEL_SCHEMA_KEY_MONITOR);
    this->blockSignals(false);
}

void ProfilePanel::updateSettings(const QString &key)
{
    switch (shash(key.toUtf8().data()))
    {
        GSETTINGS_CASE_INT_CHANGE(PANEL_SCHEMA_KEY_SIZE, Size)
        GSETTINGS_CASE_STRING_CHANGE(PANEL_SCHEMA_KEY_ORIENTATION, Orientation)
        GSETTINGS_CASE_INT_CHANGE(PANEL_SCHEMA_KEY_MONITOR, Monitor)
        GSETTINGS_CASE_DEFAULT
    }
}

}  // namespace Kiran
