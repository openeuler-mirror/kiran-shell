/**
 * Copyright (c) 2026 KylinSec Co., Ltd.
 * kiran-shell is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     Xinhao Liu <liuxinhao@kylinsec.com.cn>
 */

#pragma once

#include <QPoint>

class QScreen;
class QWidget;

namespace Kiran
{

class ShellWindow;

// 根据面板上的触发控件计算弹窗位置。
// panelGlobal: 面板窗口的屏幕坐标（顶层窗口可使用 panelWidget->pos()）
// panelOrientation: Kiran::PanelOrientation 枚举值
// triggerWidget: 触发弹窗的按钮或控件
// popupWidget: 需要定位的 ShellWindow，Wayland 下通过 ShellWindow::setPosition() 定位
void positionAppletPopup(const QPoint &panelGlobal,
                         int panelOrientation,
                         const QWidget *triggerWidget,
                         ShellWindow *popupWidget);

}  // namespace Kiran
