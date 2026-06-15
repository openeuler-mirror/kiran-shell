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

#include "popup-positioner.h"
#include "shell-window.h"

#include <qt5-log-i.h>
#include <QGSettings>
#include <QGuiApplication>
#include <QScreen>
#include <QWidget>
#include <QWindow>

#include "ks-config.h"
#include "ks-i.h"
#include "logging-category.h"

namespace Kiran
{
namespace
{

struct PopupPositionRule
{
    // 触发控件上的锚点
    QPoint anchorInTrigger;
    // 弹窗相对锚点的偏移
    QPoint popupOffset;
    // 是否需要裁剪 X 轴
    bool clampX;
    // 是否需要裁剪 Y 轴
    bool clampY;
};

bool isValidOrientation(int orientation)
{
    return orientation == Kiran::PanelOrientation::PANEL_ORIENTATION_TOP ||
           orientation == Kiran::PanelOrientation::PANEL_ORIENTATION_RIGHT ||
           orientation == Kiran::PanelOrientation::PANEL_ORIENTATION_BOTTOM ||
           orientation == Kiran::PanelOrientation::PANEL_ORIENTATION_LEFT;
}

PopupPositionRule positionRuleFor(int orientation, const QSize &triggerSize, const QSize &windowSize)
{
    switch (orientation)
    {
    case Kiran::PanelOrientation::PANEL_ORIENTATION_TOP:
        // 顶部面板：以触发控件下沿中心为弹窗上沿中心，弹窗向左偏移半个宽度。
        return {QPoint(triggerSize.width() / 2, triggerSize.height()),
                QPoint(-windowSize.width() / 2, 0), true, false};
    case Kiran::PanelOrientation::PANEL_ORIENTATION_RIGHT:
        // 右侧面板：以触发控件左沿中心为弹窗右沿中心，弹窗向左偏移一个宽度、向上偏移半个高度。
        return {QPoint(0, triggerSize.height() / 2),
                QPoint(-windowSize.width(), -windowSize.height() / 2), false, true};
    case Kiran::PanelOrientation::PANEL_ORIENTATION_BOTTOM:
        // 底部面板：以触发控件上沿中心为弹窗下沿中心，弹窗向左偏移半个宽度、向上偏移一个高度。
        return {QPoint(triggerSize.width() / 2, 0),
                QPoint(-windowSize.width() / 2, -windowSize.height()), true, false};
    case Kiran::PanelOrientation::PANEL_ORIENTATION_LEFT:
        // 左侧面板：以触发控件右沿中心为弹窗左沿中心，弹窗向上偏移半个高度。
        return {QPoint(triggerSize.width(), triggerSize.height() / 2),
                QPoint(0, -windowSize.height() / 2), false, true};
    default:
        return {QPoint(), QPoint(), false, false};
    }
}

QPoint anchorGlobalFor(const QPoint &panelGlobal,
                       const QWidget *triggerWidget,
                       const QWidget *panelWindow,
                       const PopupPositionRule &rule)
{
    // 将锚点从触发控件坐标转换到面板窗口坐标，再叠加面板全局坐标得到锚点全局坐标。
    return panelGlobal + triggerWidget->mapTo(panelWindow, rule.anchorInTrigger);
}

void clampWindowPosition(const QRect &screenGeometry,
                         const QSize &windowSize,
                         const PopupPositionRule &rule,
                         QPoint &windowPosition)
{
    if (screenGeometry.isNull())
        return;

    // 按面板方向只裁剪可能溢出的轴：上下边面板裁剪 X，左右边面板裁剪 Y。
    if (rule.clampX)
    {
        if (windowPosition.x() < screenGeometry.left())
            windowPosition.setX(screenGeometry.left());
        if (windowPosition.x() + windowSize.width() > screenGeometry.right())
            windowPosition.setX(screenGeometry.right() - windowSize.width());
    }
    if (rule.clampY)
    {
        if (windowPosition.y() < screenGeometry.top())
            windowPosition.setY(screenGeometry.top());
        if (windowPosition.y() + windowSize.height() > screenGeometry.bottom())
            windowPosition.setY(screenGeometry.bottom() - windowSize.height());
    }
}

void adjustPersonalityModeOffset(const QRect &screenGeometry, const QSize &windowSize, QPoint &windowPosition)
{
    if (windowPosition.x() == screenGeometry.x())
        windowPosition.setX(windowPosition.x() + 4);
    if (windowPosition.y() == screenGeometry.y())
        windowPosition.setY(windowPosition.y() + 4);
    if (windowPosition.x() + windowSize.width() == screenGeometry.right())
        windowPosition.setX(screenGeometry.right() - windowSize.width() - 4);
    if (windowPosition.y() + windowSize.height() == screenGeometry.bottom())
        windowPosition.setY(screenGeometry.bottom() - windowSize.height() - 4);
}

bool isPersonalityModeEnabled()
{
    QGSettings shellSettings(SHELL_SCHEMA_ID);
    return shellSettings.get(SHELL_SCHEMA_KEY_PERSONALITY_MODE).toBool();
}

QScreen *screenForAnchor(const QPoint &anchorGlobal, const QWidget *panelWindow, const ShellWindow *popupWidget)
{
    if (QScreen *screen = QGuiApplication::screenAt(anchorGlobal))
        return screen;

    if (panelWindow && panelWindow->windowHandle() && panelWindow->windowHandle()->screen())
        return panelWindow->windowHandle()->screen();

    return popupWidget ? popupWidget->screen() : nullptr;
}

QRect screenGeometryFor(const QPoint &anchorGlobal, const QWidget *panelWindow, const ShellWindow *popupWidget)
{
    // 优先使用锚点所在屏幕作为边界裁剪依据，失败时回退到面板窗口或弹窗所在屏幕。
    QScreen *screen = screenForAnchor(anchorGlobal, panelWindow, popupWidget);
    return screen ? screen->geometry() : QRect();
}

}  // namespace

void positionAppletPopup(const QPoint &panelGlobal,
                         int panelOrientation,
                         const QWidget *triggerWidget,
                         ShellWindow *popupWidget)
{
    if (!triggerWidget || !popupWidget)
    {
        KLOG_WARNING(LCLib) << "positionAppletPopup invalid args"
                            << "trigger" << triggerWidget
                            << "popup" << popupWidget;
        return;
    }

    if (!isValidOrientation(panelOrientation))
    {
        KLOG_WARNING(LCLib) << "Unknown orientation" << panelOrientation;
        return;
    }

    const QSize windowSize = popupWidget->frameSize();
    const QSize triggerSize = triggerWidget->size();
    const QWidget *panelWindow = triggerWidget->window();
    const QSize panelWindowSize = panelWindow ? panelWindow->frameSize() : QSize();

    // 根据面板方向选择触发控件上的锚点、弹窗相对锚点的偏移和需要裁剪的轴。
    const PopupPositionRule rule = positionRuleFor(panelOrientation, triggerSize, windowSize);
    const QPoint anchorGlobal = anchorGlobalFor(panelGlobal, triggerWidget, panelWindow, rule);
    const QRect screenGeometry = screenGeometryFor(anchorGlobal, panelWindow, popupWidget);

    // 将锚点全局坐标加上弹窗相对锚点的偏移得到弹窗窗口坐标。
    QPoint windowPosition = anchorGlobal + rule.popupOffset;

    // 按规则裁剪弹窗窗口坐标中可能溢出的轴。
    clampWindowPosition(screenGeometry, windowSize, rule, windowPosition);

    // personality mode 下，如果弹窗刚好贴到屏幕边缘，则向内偏移 4px。
    if (isPersonalityModeEnabled())
    {
        adjustPersonalityModeOffset(screenGeometry, windowSize, windowPosition);
    }

    KLOG_DEBUG(LCLib) << "positionAppletPopup" << popupWidget << "trigger" << triggerWidget
                       << "orientation" << panelOrientation
                       << "panelWindowSize" << panelWindowSize << "pos" << windowPosition
                       << "platform" << QGuiApplication::platformName();

    // 最终通过 ShellWindow::setPosition() 定位；Wayland 下会转为 PlasmaShellSurface::setPosition()。
    popupWidget->setPosition(windowPosition);
}

}  // namespace Kiran
