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

#include "tray-data-types.h"

QDBusArgument &operator<<(QDBusArgument &argument, const IconPixmap &icon)
{
    argument.beginStructure();
    argument << icon.width;
    argument << icon.height;
    argument << icon.bytes;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, IconPixmap &icon)
{
    argument.beginStructure();
    argument >> icon.width;
    argument >> icon.height;
    argument >> icon.bytes;
    argument.endStructure();
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const ToolTip &toolTip)
{
    argument.beginStructure();
    argument << toolTip.iconName;
    argument << toolTip.iconPixmap;
    argument << toolTip.title;
    argument << toolTip.description;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, ToolTip &toolTip)
{
    argument.beginStructure();
    argument >> toolTip.iconName;
    argument >> toolTip.iconPixmap;
    argument >> toolTip.title;
    argument >> toolTip.description;
    argument.endStructure();
    return argument;
}
