/**
 * Copyright (c) 2026 ~ 2027 KylinSec Co., Ltd.
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

#include <QFileInfo>
#include <QIcon>

namespace Kiran
{
namespace Menu
{
inline QIcon loadAppIcon(const QString &iconName)
{
    // 先按主题名解析（标准 desktop Icon=xxx 场景）。
    QIcon icon = QIcon::fromTheme(iconName);
    if (!icon.isNull())
    {
        return icon;
    }

    // 兼容 icon=xxx.png / icon=/path/to/xxx.png 的 basename 退化匹配。
    icon = QIcon::fromTheme(QFileInfo(iconName).baseName());
    if (!icon.isNull())
    {
        return icon;
    }

    // 最后兼容 Icon 字段直接给绝对路径的场景。
    QFileInfo iconFile(iconName);
    if (iconFile.exists() && iconFile.isFile())
    {
        icon = QIcon(iconFile.absoluteFilePath());
    }

    return icon;
}
}  // namespace Menu
}  // namespace Kiran
