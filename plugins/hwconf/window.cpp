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

#include <kiran-color-block.h>
#include <qt5-log-i.h>
#include <QGuiApplication>
#include <QScreen>

#include "applet.h"
#include "battery/bettery-button.h"
#include "hw-conf-window.h"
#include "ks-i.h"
#include "net/net-button.h"
#include "volume/volume-button.h"
#include "window.h"

namespace Kiran
{
namespace HwConf
{
Window::Window(IAppletImport *import, Applet *parent)
    : KiranColorBlock(parent),
      m_import(import),
      m_hwConfWindow(new HwConfWindow(this))
{
    setRadius(0);

    auto direction = getLayoutDirection();
    m_layout = new QBoxLayout(direction, this);
    m_layout->setMargin(4);

    auto volumButton = new VolumeButton(this);
    auto networkButton = new NetButton(this);
    auto powerButton = new BatteryButton(this);
    m_layout->addWidget(volumButton);
    m_layout->addWidget(networkButton);
    m_layout->addWidget(powerButton);
    hwConfButtons << volumButton << networkButton << powerButton;

    connect(volumButton, &QToolButton::clicked, this, &Window::clickHwConfButton);
    connect(networkButton, &QToolButton::clicked, this, &Window::clickHwConfButton);
    connect(powerButton, &QToolButton::clicked, this, &Window::clickHwConfButton);
    connect(m_hwConfWindow, &HwConfWindow::windowDeactivated, this, &Window::hideHwConfWindow);
    connect(m_hwConfWindow, &HwConfWindow::setVolume, volumButton, &VolumeButton::setVolume);
    connect(m_hwConfWindow, &HwConfWindow::setVolumeMute, volumButton, &VolumeButton::setVolumeMute);
    connect(volumButton, &VolumeButton::enableVolume, m_hwConfWindow, &HwConfWindow::syncVolumeEnabled);
    connect(volumButton, &VolumeButton::volumeValueChanged, m_hwConfWindow, &HwConfWindow::syncVolumeValue);
    connect(volumButton, &VolumeButton::volumeIconChanged, m_hwConfWindow, &HwConfWindow::syncVolumeIcon);
    connect(powerButton, &BatteryButton::enableBattery, m_hwConfWindow, &HwConfWindow::syncBatteryEnabled);
    connect(powerButton, &BatteryButton::batteryValueChanged, m_hwConfWindow, &HwConfWindow::syncBatteryValue);
    connect(powerButton, &BatteryButton::batteryIconChanged, m_hwConfWindow, &HwConfWindow::syncBatteryIcon);
    connect(m_hwConfWindow, &HwConfWindow::updatePosition, this, &Window::updateWindowPosition);

    QObject *Object = dynamic_cast<QObject *>(m_import->getPanel());
    connect(Object, SIGNAL(panelProfileChanged()), this, SLOT(updateLayout()));

    volumButton->updateVolume();
    powerButton->updateBattery();
}

Window::~Window()
{
}

void Window::updateLayout()
{
    // 横竖摆放
    auto direction = getLayoutDirection();
    m_layout->setDirection(direction);

    // 子控件对齐方式：左右、上下
    Qt::AlignmentFlag alignment = getLayoutAlignment();
    m_layout->setAlignment(alignment);

    auto size = m_import->getPanel()->getSize();

    if (QBoxLayout::Direction::LeftToRight == direction)
    {
        // 清理之前设置的fixed大小
        setMaximumWidth(QWIDGETSIZE_MAX);
        // 重新设置
        setFixedHeight(size);
    }
    else
    {
        setMaximumHeight(QWIDGETSIZE_MAX);
        setFixedWidth(size);
    }
}

Qt::AlignmentFlag Window::getLayoutAlignment()
{
    int orientation = m_import->getPanel()->getOrientation();
    Qt::AlignmentFlag alignment = (orientation == PanelOrientation::PANEL_ORIENTATION_BOTTOM ||
                                   orientation == PanelOrientation::PANEL_ORIENTATION_TOP)
                                      ? Qt::AlignLeft
                                      : Qt::AlignTop;

    return alignment;
}

QBoxLayout::Direction Window::getLayoutDirection()
{
    int orientation = m_import->getPanel()->getOrientation();
    auto direction = (orientation == PanelOrientation::PANEL_ORIENTATION_BOTTOM ||
                      orientation == PanelOrientation::PANEL_ORIENTATION_TOP)
                         ? QBoxLayout::Direction::LeftToRight
                         : QBoxLayout::Direction::TopToBottom;
    return direction;
}

void Window::clickHwConfButton(bool checked)
{
    if (checked)
    {
        showHwConfWindow();
    }
}

void Window::showHwConfWindow()
{
    updateWindowPosition();

    m_hwConfWindow->setVisible(true);

    for (auto button : hwConfButtons)
    {
        button->setEnabled(false);
    }
}

void Window::hideHwConfWindow()
{
    m_hwConfWindow->setVisible(false);

    for (auto button : hwConfButtons)
    {
        button->setChecked(false);
        button->setEnabled(true);
    }
}

void Window::updateWindowPosition()
{
    auto oriention = m_import->getPanel()->getOrientation();
    auto appletGeometry = geometry();
    auto windowSize = m_hwConfWindow->frameSize();
    QPoint windowPosition(0, 0);

    // 获取当前屏幕坐标
    QRect screenGeometry = QGuiApplication::primaryScreen()->geometry();
    switch (oriention)
    {
    case PanelOrientation::PANEL_ORIENTATION_TOP:
        windowPosition = mapToGlobal(appletGeometry.bottomLeft());
        // 若超出屏幕，则将窗口定位到屏幕左侧
        if (windowPosition.x() + windowSize.width() > screenGeometry.width())
        {
            windowPosition.setX(screenGeometry.width() - windowSize.width());
        }
        break;
    case PanelOrientation::PANEL_ORIENTATION_RIGHT:
        windowPosition = mapToGlobal(appletGeometry.topLeft() - QPoint(windowSize.width(), 0));
        // 若超出屏幕，则将窗口定位到屏幕底部
        if (windowPosition.y() + windowSize.height() > screenGeometry.height())
        {
            windowPosition.setY(screenGeometry.height() - windowSize.height());
        }
        break;
    case PanelOrientation::PANEL_ORIENTATION_BOTTOM:
        windowPosition = mapToGlobal(appletGeometry.topLeft() - QPoint(0, windowSize.height()));
        if (windowPosition.x() + windowSize.width() > screenGeometry.width())
        {
            windowPosition.setX(screenGeometry.width() - windowSize.width());
        }
        break;
    case PanelOrientation::PANEL_ORIENTATION_LEFT:
        windowPosition = mapToGlobal(appletGeometry.topRight());
        if (windowPosition.y() + windowSize.height() > screenGeometry.height())
        {
            windowPosition.setY(screenGeometry.height() - windowSize.height());
        }
        break;
    default:
        KLOG_WARNING() << "Unknown oriention " << oriention;
        break;
    }

    m_hwConfWindow->move(windowPosition);
}

}  // namespace HwConf
}  // namespace Kiran
