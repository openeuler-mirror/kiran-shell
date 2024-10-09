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
#include <KService/KService>
#include <QDrag>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QTreeWidgetItem>

#include "styled-tree-widget.h"

#define ROW_HEIGHT 40
#define ICON_SIZE 24
#define ICON_TEXT_MARGIN 12
#define INDENTATION 10

StyledTreeWidget::StyledTreeWidget(QWidget *parent)
    : QTreeWidget(parent)
{
    // 样式代理
    ItemDelegate *itemDelegate = new ItemDelegate(this);
    setItemDelegate(itemDelegate);

    // 为了绘制底色时，区域为完整一行，设置缩进为0，在绘制中加入缩进
    setIndentation(0);

    setMouseTracking(true);

    //    setAttribute(Qt::WA_TranslucentBackground);
    QPalette p = this->palette();
    p.setBrush(QPalette::Base, QBrush(QColor(0, 0, 0, 0)));
    setPalette(p);
}

void StyledTreeWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return)
    {
        QTreeWidgetItem *item = currentItem();
        if (item)
        {
            // 选中回车=点击
            emit itemClicked(item, 0);
        }
    }
    else
    {
        QTreeView::keyPressEvent(event);
    }
}

void StyledTreeWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        m_pressPoint = event->pos();
    }

    QTreeWidget::mousePressEvent(event);
}

void StyledTreeWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        if (qAbs(event->pos().x() - m_pressPoint.x()) < 10 && qAbs(event->pos().y() - m_pressPoint.y()) < 10)
        {
            QWidget::mouseMoveEvent(event);
            return;
        }

        QTreeWidgetItem *item = itemAt(event->pos());
        if (item)
        {
            QDrag *drag = new QDrag(this);
            QMimeData *mimeData = new QMimeData;
            KService::Ptr s = KService::serviceByMenuId(item->data(0, Qt::UserRole).toString());
            QByteArray data = QUrl::fromLocalFile(s->entryPath()).toString().toLocal8Bit();
            mimeData->setData("text/uri-list", data);
            drag->setMimeData(mimeData);
            drag->setPixmap(item->icon(0).pixmap(40, 40));
            drag->exec(Qt::CopyAction);
        }
    }

    QTreeWidget::mouseMoveEvent(event);
}

//void StyledTreeWidget::paintEvent(QPaintEvent *event)
//{
//    //    QPainter painter(this);
//    //    QColor bgColor = Qt::transparent;
//    //    painter.fillRect(rect(), Qt::red);

//    QTreeWidget::paintEvent(event);
//}

QSize ItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    size.setHeight(ROW_HEIGHT);
    return size;
}

void ItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto palette = Kiran::Theme::Palette::getDefault();

    // 选中底色
    if (option.state & QStyle::State_Selected)
    {
        QColor bgColor = palette->getColor(Kiran::Theme::Palette::SELECTED, Kiran::Theme::Palette::WIDGET);

        painter->fillRect(option.rect, bgColor);
    }
    // 鼠标移入底色
    if (option.state & QStyle::State_MouseOver)
    {
        QColor bgColor = palette->getColor(Kiran::Theme::Palette::MOUSE_OVER, Kiran::Theme::Palette::WIDGET);
        painter->fillRect(option.rect, bgColor);
    }

    QString text = index.data(Qt::DisplayRole).toString();
    QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));

    QRect baseRect = option.rect;
    // 缩进
    baseRect.adjust(INDENTATION, 0, INDENTATION, 0);

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

        //此时图标在左上，移到左中
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
    painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, text);
}
