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

#pragma once

#include <QObject>
#include "src/shell/profile/profile-utils.h"

class QGSettings;

namespace Kiran
{
class ProfileApplet : public QObject
{
    Q_OBJECT

    GSETTINGS_PROPERTY_STRING_DECLARATION(id, ID);
    GSETTINGS_PROPERTY_STRING_DECLARATION(panel, Panel);
    GSETTINGS_PROPERTY_INT_DECLARATION(position, Position)
    GSETTINGS_PROPERTY_BOOLEAN_DECLARATION(panelRightStick, PanelRightStick)

public:
    ProfileApplet(const QString& uid);
    // 唯一ID
    QString getUID()
    {
        return this->m_uid;
    };

private:
    void init();
    void initSettings();

private Q_SLOTS:
    void updateSettings(const QString& key);

private:
    // 面板配置
    QGSettings* m_settings;
    QString m_uid;
};
}  // namespace Kiran
