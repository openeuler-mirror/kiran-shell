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
#include <kiran-integration/theme/palette.h>
#include <QPainter>
#include <QPainterPath>
#include <QStyledItemDelegate>

#include "lib/common/utility.h"

namespace Kiran
{
namespace Menu
{
enum
{
    ROW_HEIGHT = 40,
    ICON_SIZE = 24,
    ICON_TEXT_MARGIN = 12,
    INDENTATION = 10
};

class TreeDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    TreeDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
    {
    }
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QSize size = QStyledItemDelegate::sizeHint(option, index);
        size.setHeight(ROW_HEIGHT);
        return size;
    }
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        painter->setRenderHint(QPainter::Antialiasing, true);
        auto *palette = Kiran::Theme::Palette::getDefault();
        painter->setPen(Qt::NoPen);

        QRect bgRect = option.rect;
        bgRect.adjust(0, 0, -8, 0);
        // 选中底色（仅在有焦点时绘制，焦点移出后不保留选中样式）
        if ((option.state & QStyle::State_Selected) && (option.state & QStyle::State_HasFocus))
        {
            QColor bgColor = palette->getColor(Kiran::Theme::Palette::SELECTED, Kiran::Theme::Palette::WIDGET);
            //            painter->fillRect(bgRect, bgColor);
            painter->setBrush(bgColor);
            QPainterPath path;
            path.addRoundedRect(bgRect, 5, 5);
            painter->drawPath(path);
        }
        // 鼠标移入底色
        if (option.state & QStyle::State_MouseOver)
        {
            QColor bgColor = palette->getColor(Kiran::Theme::Palette::MOUSE_OVER, Kiran::Theme::Palette::WIDGET);
            //            painter->fillRect(bgRect, bgColor);
            painter->setBrush(bgColor);
            QPainterPath path;
            path.addRoundedRect(bgRect, 4, 4);
            painter->drawPath(path);
        }

        QString text = index.data(Qt::DisplayRole).toString();
        QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));

        QRect baseRect = option.rect;
        // 缩进
        baseRect.adjust(INDENTATION, 0, -INDENTATION, 0);

        QRect iconRect = baseRect;

        if (!icon.isNull())
        {
            // 分类图标有点大，略微缩小
            if (!index.parent().isValid())
            {
                iconRect.setSize(QSize(ICON_SIZE - 5, ICON_SIZE - 5));
            }
            else
            {
                iconRect.setSize(QSize(ICON_SIZE, ICON_SIZE));
            }

            // 此时图标在左上，移到左中
            int yAdjust = (option.rect.height() - iconRect.height()) / 2;
            if (!index.parent().isValid())
            {
                iconRect.adjust(0, yAdjust, 0, yAdjust);
            }
            else
            {
                // 子节点再次缩进
                iconRect.adjust(INDENTATION, yAdjust, INDENTATION, yAdjust);
            }

            icon.paint(painter, iconRect, Qt::AlignCenter);
        }

        QRect textRect = baseRect;
        textRect.adjust(iconRect.right() - baseRect.x() + ICON_TEXT_MARGIN, 0, 0, 0);
        text = Utility::getElidedText(painter->fontMetrics(), text, textRect.width());
        // 文字颜色
        painter->setPen(palette->getColor(Kiran::Theme::Palette::NORMAL, Kiran::Theme::Palette::TEXT));
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, text);
    }
};
}  // namespace Menu
}  // namespace Kiran
