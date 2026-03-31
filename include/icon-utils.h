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

inline QIcon loadIconByKIconLoader(const QString &name)
{
    if (name.isEmpty())
    {
        return QIcon();
    }
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

inline QIcon loadIcon(const QString &iconName)
{
    if (iconName.isEmpty())
    {
        return QIcon();
    }

    QIcon icon = QIcon::fromTheme(iconName);
    if (!icon.isNull())
    {
        return icon;
    }

    icon = loadIconByKIconLoader(iconName);
    if (!icon.isNull())
    {
        return icon;
    }

    const QString baseName = QFileInfo(iconName).baseName();
    icon = QIcon::fromTheme(baseName);
    if (!icon.isNull())
    {
        return icon;
    }

    if (baseName != iconName)
    {
        icon = loadIconByKIconLoader(baseName);
        if (!icon.isNull())
        {
            return icon;
        }
    }

    QFileInfo iconFile(iconName);
    if (iconFile.exists() && iconFile.isFile())
    {
        icon = QIcon(iconFile.absoluteFilePath());
    }

    return icon;
}
}  // namespace Kiran
