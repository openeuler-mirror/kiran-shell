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

#include <ks-i.h>
#include <plugin-i.h>
#include <QBoxLayout>

#include "app-previewer.h"
#include "lib/common/utility.h"

namespace Kiran
{
namespace Taskbar
{
AppPreviewer::AppPreviewer(IAppletImport *import, QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint | Qt::Tool),
      m_import(import)
{
    //横竖摆放
    auto direction = getLayoutDirection();
    m_layout = new QBoxLayout(direction, this);
    setLayout(m_layout);
    m_layout->setMargin(0);
    m_layout->setSpacing(3);

    //子控件排列方式：左右、上下
    Qt::AlignmentFlag alignment = getLayoutAlignment();
    m_layout->setAlignment(alignment);

    updateLayout();
}

QBoxLayout::Direction AppPreviewer::getLayoutDirection()
{
    int orientation = m_import->getPanel()->getOrientation();
    auto direction = (orientation == PanelOrientation::PANEL_ORIENTATION_BOTTOM ||
                      orientation == PanelOrientation::PANEL_ORIENTATION_TOP)
                         ? QBoxLayout::Direction::LeftToRight
                         : QBoxLayout::Direction::TopToBottom;
    return direction;
}

Qt::AlignmentFlag AppPreviewer::getLayoutAlignment()
{
    int orientation = m_import->getPanel()->getOrientation();
    Qt::AlignmentFlag alignment = (orientation == PanelOrientation::PANEL_ORIENTATION_BOTTOM ||
                                   orientation == PanelOrientation::PANEL_ORIENTATION_TOP)
                                      ? Qt::AlignLeft
                                      : Qt::AlignTop;

    return alignment;
}

void AppPreviewer::updateLayout()
{
    if (m_mapWindowPreviewers.isEmpty())
    {
        return;
    }

    auto previwer = m_mapWindowPreviewers.begin().value();
    setFixedSize(previwer->width() * m_mapWindowPreviewers.size(), previwer->height());

    Utility::clearLayout(m_layout);
    for (WindowPreviewer *previwer : m_mapWindowPreviewers)
    {
        m_layout->addWidget(previwer);
    }
}

void AppPreviewer::addWindow(WId wid)
{
    m_mapWindowPreviewers[wid] = new WindowPreviewer(wid, this);
    connect(m_mapWindowPreviewers[wid], &WindowPreviewer::closeWindow, this, &AppPreviewer::closeWindow);
    connect(m_mapWindowPreviewers[wid], &WindowPreviewer::hideWindow, this, &QWidget::hide, Qt::DirectConnection);

    int panelSize = m_import->getPanel()->getSize();
    m_mapWindowPreviewers[wid]->setFixedSize(panelSize * 4, panelSize * 4);
    updateLayout();
}

void AppPreviewer::removeWindow(WId wid)
{
    WindowPreviewer *previewr = m_mapWindowPreviewers.take(wid);
    if (previewr)
    {
        delete previewr;
        previewr = nullptr;
    }
    updateLayout();
}

void AppPreviewer::showEvent(QShowEvent *event)
{
    for (WindowPreviewer *previwer : m_mapWindowPreviewers)
    {
        //更新截图
        previwer->updatePreviewer();
    }
}

void AppPreviewer::leaveEvent(QEvent *event)
{
    bool checkCanHide = false;
    for (WindowPreviewer *previwer : m_mapWindowPreviewers)
    {
        if (!(checkCanHide = previwer->checkCanHide()))
        {
            break;
        }
    }

    if (checkCanHide)
    {
        hide();
    }
}

}  // namespace Taskbar

}  // namespace Kiran
