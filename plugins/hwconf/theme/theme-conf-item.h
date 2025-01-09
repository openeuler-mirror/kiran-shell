/**
 * Copyright (c) 2024 ~ 2025 KylinSec Co., Ltd.
 * kiran-shell is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     yangfeng <yangfeng@kylinsec.com.cn>
 */

#pragma once

#include "hw-conf-item.h"

class KSAppearance;
namespace Kiran
{
namespace HwConf
{
enum AppearanceThemeType
{
    // META主题，META主题包括了GTK/图标/光标等主题的设置
    APPEARANCE_THEME_TYPE_META = 0,
    // GTK主题（依赖GTK的版本）
    APPEARANCE_THEME_TYPE_GTK,
    // metacity主题
    APPEARANCE_THEME_TYPE_METACITY,
    // 图标主题
    APPEARANCE_THEME_TYPE_ICON,
    // 光标主题
    APPEARANCE_THEME_TYPE_CURSOR,
    APPEARANCE_THEME_TYPE_LAST,
};

class ThemeConfItem : public HwConfItem
{
    Q_OBJECT
public:
    ThemeConfItem(QWidget* parent = nullptr);

    void init();

private:
    void serviceOwnerChanged(const QString& service, const QString& oldOwner, const QString& newOwner);
    void themeIconClicked();

    bool getTheme(QString& themeID);
    bool setTheme(const QString& themeID);

signals:
    void enableTheme(bool enabled);

private:
    KSAppearance* m_ksAppearance;

    QString m_themeID;
};
}  // namespace HwConf
}  // namespace Kiran
