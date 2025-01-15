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
#include <QGuiApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

#include "lib/common/utility.h"
#include "styled-button.h"

StyledButton::StyledButton(QWidget *parent)
    : QToolButton(parent),
      m_hovered(false),
      m_pressed(false)
{
    setCheckable(true);
    // 启用悬浮事件
    setAttribute(Qt::WA_Hover);
}

void StyledButton::setTextColor(QColor color)
{
    m_textColor = color;
}

void StyledButton::enterEvent(QEvent *event)
{
    m_hovered = true;

    //    QToolButton::enterEvent(event);
}

void StyledButton::leaveEvent(QEvent *event)
{
    m_hovered = false;

    //    QToolButton::leaveEvent(event);
}

void StyledButton::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_pressed = true;
        update();
    }

    QToolButton::mousePressEvent(event);
}

void StyledButton::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_pressed = false;
        update();
    }

    QToolButton::mouseReleaseEvent(event);
}

void StyledButton::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);  // 设置反走样，使边缘平滑

    auto palette = Kiran::Theme::Palette::getDefault();

    // 背景绘制
    QColor bgColor;
    if (isChecked())
    {
        // 选中
        bgColor = palette->getColor(Kiran::Theme::Palette::SUNKEN, Kiran::Theme::Palette::WIDGET);
    }

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
        bgColor = Qt::transparent;
    }

    if (!isEnabled())
    {
        bgColor = Qt::transparent;
    }

    painter.setBrush(bgColor);

    QPainterPath path;
    path.addRoundedRect(rect(), 4, 4);
    QPen pen = painter.pen();
    painter.setPen(Qt::NoPen);

    painter.drawPath(path);

    painter.setPen(pen);
    if (m_textColor.isValid())
    {
        painter.setPen(m_textColor);
    }
    // 图标文字绘制
    switch (toolButtonStyle())
    {
    // 仅图标、仅文字、图标+文字并列
    case Qt::ToolButtonIconOnly:
    case Qt::ToolButtonTextOnly:
    case Qt::ToolButtonTextBesideIcon:
    {
        const int margin = 3;
        int iconWH = iconSize().width();  // 此处使用iconSize，使用时能灵活设置
        if (0 == iconWH)
        {
            iconWH = height() / 2;
        }
        int textWidth = fontMetrics().horizontalAdvance(text());
        int textHeight = fontMetrics().height();

        if (!icon().isNull())
        {
            int x = (width() - iconWH - (textWidth > 0 ? (textWidth + margin) : 0)) / 2;
            int y = (height() - iconWH) / 2;
            QPixmap pixmap = icon().pixmap(iconWH, iconWH);
            painter.drawPixmap(x, y, iconWH, iconWH, pixmap);
        }
        if (!text().isEmpty())
        {
            int x = (width() - textWidth) / 2;
            x += !icon().isNull() ? (iconWH + margin) : 0;
            int y = (height() - textHeight) / 2;
            painter.drawText(x, y, textWidth, textHeight, Qt::AlignCenter, text());
        }
        break;
    }
    // 文字位于图标下方
    case Qt::ToolButtonTextUnderIcon:
    {
        // 图标起始位置(1/4,1/8),宽高为1/2
        // 文字起始位置(1/8,3/4),宽为3/4
        QRect rectIcon = rect();
        rectIcon.adjust(rect().width() / 4, rect().height() / 8, 0, 0);
        rectIcon.setSize(QSize(rect().width() / 2, rect().height() / 2));

        QRect rectText = rect();
        rectText.adjust(rect().width() / 8, rect().height() / 4 * 3, 0, 0);
        rectText.setSize(QSize(rect().width() / 4 * 3, fontMetrics().height()));

        QPixmap pixmap = icon().pixmap(rectIcon.size());
        painter.drawPixmap(rectIcon, pixmap);

        QString elideText = Utility::getElidedText(fontMetrics(), text(), rectText.width());
        painter.drawText(rectText, Qt::AlignCenter, elideText);

        break;
    }
    default:
    {
        break;
    }
    }
}
