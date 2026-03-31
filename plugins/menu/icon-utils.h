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

#include <KIconThemes/KIconLoader>
#include <QFileInfo>
#include <QIcon>
#include <QPixmap>

namespace Kiran
{
namespace Menu
{

inline QIcon iconFromKIconLoader(const QString &name)
{
    if (name.isEmpty())
    {
        return QIcon();
    }
    // KIconLoader 解决 QIcon::fromTheme 无法命中新安装的主题图标的问题。
    // 做为补充，优先使用 QIcon::fromTheme
    QPixmap pixmap = KIconLoader::global()->loadIcon(name,
                                                     KIconLoader::Desktop,
                                                     0,
                                                     KIconLoader::DefaultState,
                                                     QStringList(),
                                                     nullptr,
                                                     true);
    if (pixmap.isNull())
    {
        return QIcon();
    }
    return QIcon(pixmap);
}

inline QIcon loadAppIcon(const QString &iconName)
{
    if (iconName.isEmpty())
    {
        return QIcon();
    }

    // 先按主题名解析（标准 desktop Icon=xxx 场景）。
    QIcon icon = QIcon::fromTheme(iconName);
    if (!icon.isNull())
    {
        return icon;
    }

    icon = iconFromKIconLoader(iconName);
    if (!icon.isNull())
    {
        return icon;
    }

    // 兼容 icon=xxx.png / icon=/path/to/xxx.png 的 basename 退化匹配。
    const QString baseName = QFileInfo(iconName).baseName();
    icon = QIcon::fromTheme(baseName);
    if (!icon.isNull())
    {
        return icon;
    }

    if (baseName != iconName)
    {
        icon = iconFromKIconLoader(baseName);
        if (!icon.isNull())
        {
            return icon;
        }
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
