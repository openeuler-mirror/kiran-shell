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

#include <kiran-integration/theme/palette.h>
#include <ks-i.h>
#include <qt5-log-i.h>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QToolTip>

#include "calendar-button.h"

namespace Kiran
{
namespace Calendar
{
CalendarButton::CalendarButton(IAppletImport *import, QWidget *parent)
    : QPushButton(parent),
      m_import(import),
      m_hovered(false),
      m_pressed(false)
{
    setFlat(true);
    setCheckable(true);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

void CalendarButton::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_pressed = true;
    }

    QPushButton::mousePressEvent(event);
}

void CalendarButton::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_pressed = false;
    }

    QPushButton::mouseReleaseEvent(event);
}

void CalendarButton::enterEvent(QEvent *event)
{
    Q_UNUSED(event);

    QToolTip::showText(mapToGlobal(geometry().bottomLeft()), toolTip(), this);

    m_hovered = true;
}

void CalendarButton::leaveEvent(QEvent *event)
{
    m_hovered = false;
}

void CalendarButton::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);  // 设置反走样，使边缘平滑

    auto palette = Kiran::Theme::Palette::getDefault();

    // 背景绘制
    QColor bgColor;

    if (isChecked())
    {
        // 选中
        bgColor = palette->getColor(Kiran::Theme::Palette::SELECTED, Kiran::Theme::Palette::WIDGET);
    }

    if (m_hovered)
    {
        // 悬停
        bgColor = palette->getColor(Kiran::Theme::Palette::MOUSE_OVER, Kiran::Theme::Palette::WIDGET);
    }
    else
    {
        // 正常
        bgColor = Qt::transparent;
    }

    QPainterPath path;
    path.addRoundedRect(rect(), 4, 4);
    painter.setBrush(bgColor);

    QPen pen = painter.pen();
    painter.setPen(Qt::NoPen);

    painter.drawPath(path);

    //    painter.fillRect(rect(), bgColor);

    QColor penColor = palette->getBaseColors().baseForeground;

    painter.setPen(penColor);
    painter.drawText(rect(), Qt::AlignCenter, text());
}

}  // namespace Calendar
}  // namespace Kiran
