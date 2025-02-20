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
#include <QPainter>
#include <QPainterPath>

#include "ks-i.h"
#include "workspace-add-btn.h"

namespace Kiran
{
namespace Workspace
{
WorkspaceAddBtn::WorkspaceAddBtn(QWidget *parent)
    : StyledButton(parent)
{
    setIcon(QIcon::fromTheme(KS_ICON_WORKSPACE_PLUS_SYMBOLIC));
}

void WorkspaceAddBtn::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);  // 设置反走样，使边缘平滑

    auto *palette = Kiran::Theme::Palette::getDefault();
    QColor bgColor;
    if (m_pressed)
    {
        // 点击
        bgColor = palette->getColor(Kiran::Theme::Palette::SUNKEN, Kiran::Theme::Palette::WIDGET);
        bgColor.setAlpha(180);
    }
    else if (m_hovered)
    {
        // 悬停
        bgColor = palette->getColor(Kiran::Theme::Palette::MOUSE_OVER, Kiran::Theme::Palette::WIDGET);
        bgColor.setAlpha(150);
    }
    else
    {
        // 正常
        bgColor = Qt::transparent;
    }

    painter.setBrush(bgColor);

    QPen pen = painter.pen();
    painter.setPen(Qt::NoPen);

    QPainterPath path;
    path.addRoundedRect(rect(), 4, 4);
    painter.drawPath(path);

    int iconWH = iconSize().width();
    int iconX = (width() - iconWH) / 2;
    int iconY = (height() - iconWH) / 2;
    QPixmap pixmap = icon().pixmap(iconWH, iconWH);
    painter.drawPixmap(iconX, iconY, iconWH, iconWH, pixmap);
}
}  // namespace Workspace
}  // namespace Kiran
