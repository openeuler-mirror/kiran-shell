/**
 * Copyright (c) 2020 ~ 2023 KylinSec Co., Ltd.
 * kiran-control-panel is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     liuxinhao <liuxinhao@kylinsec.com.cn>
 */
#pragma once
#include <QtDBus/QtDBus>

class IdleAction
{
public:
    IdleAction(int timeout = 0, int action = 0)
        : idleTimeout(timeout),
          idleAction(action)
    {
    }
    int idleTimeout;
    int idleAction;
    friend QDBusArgument &operator<<(QDBusArgument &arg, const IdleAction &action)
    {
        arg.beginStructure();
        arg << action.idleTimeout;
        arg << action.idleAction;
        arg.endStructure();
        return arg;
    };
    friend const QDBusArgument &operator>>(const QDBusArgument &arg, IdleAction &action)
    {
        arg.beginStructure();
        arg >> action.idleTimeout;
        arg >> action.idleAction;
        arg.endStructure();
        return arg;
    };
};
Q_DECLARE_METATYPE(IdleAction)