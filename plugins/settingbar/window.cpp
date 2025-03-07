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

#include <kiran-integration/theme/palette.h>
#include <qt5-log-i.h>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

#include "applet.h"
#include "battery/bettery-button.h"
#include "ks-i.h"
#include "lib/common/utility.h"
#include "net/net-button.h"
#include "setting-window.h"
#include "volume/volume-button.h"
#include "window.h"

namespace Kiran
{
namespace SettingBar
{
Window::Window(IAppletImport *import, Applet *parent)
    : KiranColorBlock(parent),
      m_import(import),
      m_hwConfWindow(new SettingWindow(this))
{
    setRadius(0);
    setAttribute(Qt::WA_Hover);

    auto direction = getLayoutDirection();
    m_layout = new QBoxLayout(direction, this);
    m_layout->setMargin(4);
    m_layout->setSpacing(0);

    auto *volumButton = new VolumeButton(this);
    auto *networkButton = new NetButton(this);
    auto *powerButton = new BatteryButton(this);
    m_layout->addWidget(volumButton);
    m_layout->addWidget(networkButton);
    m_layout->addWidget(powerButton);
    hwConfButtons << volumButton << networkButton << powerButton;

    connect(volumButton, &QToolButton::clicked, this, &Window::clickHwConfButton);
    connect(networkButton, &QToolButton::clicked, this, &Window::clickHwConfButton);
    connect(powerButton, &QToolButton::clicked, this, &Window::clickHwConfButton);
    connect(m_hwConfWindow, &SettingWindow::windowDeactivated, this, &Window::hideHwConfWindow);
    connect(m_hwConfWindow, &SettingWindow::setVolume, volumButton, &VolumeButton::setVolume);
    connect(m_hwConfWindow, &SettingWindow::setVolumeMute, volumButton, &VolumeButton::setVolumeMute);
    connect(volumButton, &VolumeButton::enableVolume, m_hwConfWindow, &SettingWindow::syncVolumeEnabled);
    connect(volumButton, &VolumeButton::volumeValueChanged, m_hwConfWindow, &SettingWindow::syncVolumeValue);
    connect(volumButton, &VolumeButton::volumeIconChanged, m_hwConfWindow, &SettingWindow::syncVolumeIcon);
    connect(powerButton, &BatteryButton::enableBattery, m_hwConfWindow, &SettingWindow::syncBatteryEnabled);
    connect(powerButton, &BatteryButton::batteryValueChanged, m_hwConfWindow, &SettingWindow::syncBatteryValue);
    connect(powerButton, &BatteryButton::batteryIconChanged, m_hwConfWindow, &SettingWindow::syncBatteryIcon);
    connect(m_hwConfWindow, &SettingWindow::updatePosition, this, &Window::updateWindowPosition);

    auto *panelObject = dynamic_cast<QObject *>(m_import->getPanel());
    connect(panelObject, SIGNAL(panelProfileChanged()), this, SLOT(updateLayout()));

    volumButton->init();
    powerButton->init();
}

void Window::enterEvent(QEvent *event)
{
    m_hovered = true;
}

void Window::leaveEvent(QEvent *event)
{
    m_hovered = false;
}

void Window::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_pressed = true;
        update();
    }

    KiranColorBlock::mousePressEvent(event);
}

void Window::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_pressed = false;
        update();
    }

    KiranColorBlock::mouseReleaseEvent(event);
}

void Window::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);

    auto *palette = Kiran::Theme::Palette::getDefault();

    QColor bgColor = palette->getBaseColors().containerBackground;
    painter.setBrush(bgColor);
    painter.drawRect(rect());

    if (m_pressed)
    {
        // 点击
        bgColor = palette->getColor(Kiran::Theme::Palette::SUNKEN, Kiran::Theme::Palette::WIDGET);
    }
    else if (m_hovered)
    {
        // 悬停
        bgColor = palette->getColor(Kiran::Theme::Palette::MOUSE_OVER, Kiran::Theme::Palette::WIDGET);
    }
    else
    {
        // 正常
        bgColor = palette->getBaseColors().containerBackground;
    }
    painter.setBrush(bgColor);

    QPainterPath path;
    auto windowRect = rect();
    windowRect.adjust(4, 4, -4, -4);
    path.addRoundedRect(windowRect, 4, 4);

    painter.drawPath(path);
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
    m_hwConfWindow->setVisible(true);

    for (auto *button : hwConfButtons)
    {
        button->setEnabled(false);
    }

    updateWindowPosition();
}

void Window::hideHwConfWindow()
{
    m_hwConfWindow->setVisible(false);

    for (auto *button : hwConfButtons)
    {
        button->setChecked(false);
        button->setEnabled(true);
    }
}

void Window::updateWindowPosition()
{
    auto oriention = m_import->getPanel()->getOrientation();
    auto *screen = m_import->getPanel()->getScreen();
    Utility::updatePopWidgetPos(screen, oriention, this, m_hwConfWindow);
}

}  // namespace SettingBar
}  // namespace Kiran
