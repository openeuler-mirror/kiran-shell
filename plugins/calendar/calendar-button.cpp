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
#include <qt5-log-i.h>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QToolTip>

#include "calendar-button.h"
#include "ks-i.h"

namespace Kiran
{
namespace Calendar
{
CalendarButton::CalendarButton(IAppletImport *import, QWidget *parent)
    : QPushButton(parent),
      m_import(import)
{
    setFlat(true);
    setCheckable(true);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

// 根据按钮当前尺寸和文本内容动态计算合适的显示字体
// 算法：基于可用高度和行数计算初始字号，若文本宽度超出则逐级缩小
// 字号限制在 12~24 像素范围内，确保可读性和美观性
QFont CalendarButton::calcDisplayFont() const
{
    QStringList lines = text().split('\n');
    int lineCount = qMax(1, lines.size());
    int verticalPadding = qMax(4, height() / 10);
    int horizontalPadding = qMax(8, width() / 12);
    int availableHeight = qMax(1, height() - verticalPadding * 2);
    int availableWidth = qMax(1, width() - horizontalPadding * 2);

    int pixelSize = qBound(12, availableHeight * 3 / (lineCount * 4), 24);

    QFont buttonFont("Noto Sans CJK SC");
    buttonFont.setPixelSize(pixelSize);
    QFontMetrics metrics(buttonFont);

    auto calcMaxLineWidth = [&]()
    {
        int maxLineWidth = 0;
        for (const auto &line : lines)
        {
            maxLineWidth = qMax(maxLineWidth, metrics.horizontalAdvance(line));
        }
        return maxLineWidth;
    };

    while (buttonFont.pixelSize() > 12 && calcMaxLineWidth() > availableWidth)
    {
        buttonFont.setPixelSize(buttonFont.pixelSize() - 1);
        metrics = QFontMetrics(buttonFont);
    }

    return buttonFont;
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

    m_hovered = true;

    QToolTip::showText(QCursor::pos(), toolTip(), this, QRect(), 1500);
}

void CalendarButton::leaveEvent(QEvent *event)
{
    m_hovered = false;
}

void CalendarButton::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);  // 设置反走样，使边缘平滑

    auto *palette = Kiran::Theme::Palette::getDefault();

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

    painter.setPen(Qt::NoPen);
    painter.drawPath(path);

    QColor penColor = palette->getBaseColors().baseForeground;
    painter.setPen(penColor);
    painter.setFont(calcDisplayFont());
    painter.drawText(rect(), Qt::AlignCenter, text());
}

}  // namespace Calendar
}  // namespace Kiran
