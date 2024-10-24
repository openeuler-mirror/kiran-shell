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

#include <QObject>

namespace Kiran
{
namespace Workspace
{
// 操作工作区的接口,主要是对kf5关于工作区的封装,以及对x11和wayland一些工作区区别操作的封装
class DesktopHelper : public QObject
{
    Q_OBJECT
public:
    explicit DesktopHelper(QObject *parent = nullptr);

    static int numberOfDesktops();
    static int currentDesktop();
    static void setCurrentDesktop(int desktop);
    static void createDesktop();
    static void removeDesktop(int deskToRemove);
signals:
    void currentDesktopChanged(int desktop);
    void numberOfDesktopsChanged(int num);
};
}  // namespace Workspace
}  // namespace Kiran
