/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
 * kiran-session-manager is licensed under Mulan PSL v2.
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

#include <kiran-style/style-palette.h>
#include <QPainter>

#include "style-app-button.h"

StyleAppButton::StyleAppButton(QWidget *parent)
    : QPushButton(parent),
      m_hovered(false)
{
    setCheckable(true);
    setMouseTracking(true);
}

void StyleAppButton::enterEvent(QEvent *event)
{
    m_hovered = true;
}

void StyleAppButton::leaveEvent(QEvent *event)
{
    m_hovered = false;
}

void StyleAppButton::paintEvent(QPaintEvent *event)
{
    auto stylePalette = Kiran::StylePalette::instance();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);  // 设置反走样，使边缘平滑

    QColor bgColor;  // 背景色
    if (isChecked())
    {
        // 选中
        bgColor = stylePalette->color(Kiran::StylePalette::ColorState::Active,
                                      Kiran::StylePalette::WidgetType::Widget,
                                      Kiran::StylePalette::WidgetColorRule::Background);  // 选中
    }
    else if (m_hovered)
    {
        // 悬停
        bgColor = stylePalette->color(Kiran::StylePalette::ColorState::Hover,
                                      Kiran::StylePalette::WidgetType::Widget,
                                      Kiran::StylePalette::WidgetColorRule::Background);
    }
    else
    {
        // 正常
        bgColor = Qt::transparent;
    }

    painter.fillRect(rect(), bgColor);

    const int margin = 3;

    int iconWidth = iconSize().width();
    int iconHeight = iconSize().height();
    int textWidth = fontMetrics().horizontalAdvance(text());
    int x = (width() - iconWidth - (textWidth > 0 ? (textWidth + margin) : 0)) / 2;
    int y = (height() - iconHeight) / 2;
    QPixmap pixmap = icon().pixmap(iconSize());
    painter.drawPixmap(x, y, iconWidth, iconHeight, pixmap);

    painter.drawText(x + iconWidth + margin, y, textWidth, iconHeight, Qt::AlignCenter, text());
}
