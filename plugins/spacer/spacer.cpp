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

#include <kiran-color-block.h>
#include <ks-i.h>
#include <QPainter>

#include "spacer.h"

namespace Kiran
{
Spacer::Spacer(IAppletImport *import)
    : m_import(import)
{
    auto direction = getLayoutDirection();
    m_layout = new QBoxLayout(direction, this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(4);
    setLayout(m_layout);
    m_prevBlock = new KiranColorBlock(this);
    m_nextBlock = new KiranColorBlock(this);
    m_layout->addWidget(m_prevBlock);
    m_layout->addWidget(m_nextBlock);

    QObject *Object = dynamic_cast<QObject *>(m_import->getPanel());
    connect(Object, SIGNAL(panelProfileChanged()), this, SLOT(updateLayout()));

    updateLayout();
}

void Spacer::updateLayout()
{
    int orientation = m_import->getPanel()->getOrientation();
    auto size = m_import->getPanel()->getSize();
    auto direction = getLayoutDirection();
    m_layout->setDirection(direction);

    if (orientation == PanelOrientation::PANEL_ORIENTATION_BOTTOM ||
        orientation == PanelOrientation::PANEL_ORIENTATION_TOP)
    {
        setFixedSize(20, size);
        m_prevBlock->setRoundedCorner(KiranColorBlock::CornersRight);
        m_nextBlock->setRoundedCorner(KiranColorBlock::CornersLeft);
    }
    else
    {
        setFixedSize(size, 20);
        m_prevBlock->setRoundedCorner(KiranColorBlock::CornersBottom);
        m_nextBlock->setRoundedCorner(KiranColorBlock::CornersTop);
    }
}

Qt::AlignmentFlag Spacer::getLayoutAlignment()
{
    int orientation = m_import->getPanel()->getOrientation();
    Qt::AlignmentFlag alignment = (orientation == PanelOrientation::PANEL_ORIENTATION_BOTTOM ||
                                   orientation == PanelOrientation::PANEL_ORIENTATION_TOP)
                                      ? Qt::AlignLeft
                                      : Qt::AlignTop;

    return alignment;
}

QBoxLayout::Direction Spacer::getLayoutDirection()
{
    int orientation = m_import->getPanel()->getOrientation();
    auto direction = (orientation == PanelOrientation::PANEL_ORIENTATION_BOTTOM ||
                      orientation == PanelOrientation::PANEL_ORIENTATION_TOP)
                         ? QBoxLayout::Direction::LeftToRight
                         : QBoxLayout::Direction::TopToBottom;
    return direction;
}
}  // namespace Kiran
