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
 * Author:     yangfeng <yangfeng@kylinsec.com.cn>
 */

#pragma once

#include <QDBusArgument>

struct IconPixmap
{
    int width;
    int height;
    QByteArray bytes;
};

using IconPixmapVector = QVector<IconPixmap>;

struct ToolTip
{
    QString iconName;
    IconPixmapVector iconPixmap;
    QString title;
    QString description;
};

QDBusArgument &operator<<(QDBusArgument &argument, const IconPixmap &icon);
const QDBusArgument &operator>>(const QDBusArgument &argument, IconPixmap &icon);

QDBusArgument &operator<<(QDBusArgument &argument, const ToolTip &toolTip);
const QDBusArgument &operator>>(const QDBusArgument &argument, ToolTip &toolTip);

Q_DECLARE_METATYPE(IconPixmap)
Q_DECLARE_METATYPE(IconPixmapVector)
Q_DECLARE_METATYPE(ToolTip)
