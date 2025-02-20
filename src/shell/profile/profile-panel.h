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

#include <QObject>
#include "profile-utils.h"

class QGSettings;

namespace Kiran
{
class ProfilePanel : public QObject
{
    Q_OBJECT
    // 面板高度（纵向摆放则代表宽度）
    GSETTINGS_PROPERTY_INT_DECLARATION(size, Size)
    // 面板在四周的位置
    GSETTINGS_PROPERTY_STRING_DECLARATION(orientation, Orientation)
    // 显示在第几个显示器中
    GSETTINGS_PROPERTY_INT_DECLARATION(monitor, Monitor)

public:
    ProfilePanel(QString uid);
    // 唯一ID
    QString getUID()
    {
        return m_uid;
    };

private:
    void init();
    void initSettings();

private slots:
    void updateSettings(const QString& key);

private:
    // 面板配置
    QGSettings* m_settings;
    QString m_uid;
};
}  // namespace Kiran
