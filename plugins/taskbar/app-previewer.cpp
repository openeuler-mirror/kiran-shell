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

#include <ks-i.h>
#include <plugin-i.h>
#include <qt5-log-i.h>
#include <KWindowSystem/NETWM>
#include <QBoxLayout>

#include "app-group.h"
#include "app-previewer.h"
#include "lib/common/define.h"
#include "lib/common/setting-process.h"
#include "lib/common/utility.h"
#include "lib/common/window-info-helper.h"

#define PREVIEWER_SPACING 3

namespace Kiran
{
namespace Taskbar
{
AppPreviewer::AppPreviewer(IAppletImport *import, AppGroup *parent)
    : QWidget(parent, Qt::FramelessWindowHint | Qt::Tool),
      m_import(import)
{
    connect(parent, &AppGroup::windowAdded, this, &AppPreviewer::addWindow);
    connect(parent, &AppGroup::windowRemoved, this, &AppPreviewer::removeWindow);
    connect(parent, &AppGroup::windowChanged, this, &AppPreviewer::windowChanged);
    connect(parent, &AppGroup::activeWindowChanged, this, &AppPreviewer::activeWindowChanged);

    connect(parent, &AppGroup::previewerShow, this, &AppPreviewer::showPreviewer);
    connect(parent, &AppGroup::previewerHide, this, &AppPreviewer::hidePreviewer);

    // 横竖摆放
    auto direction = getLayoutDirection();
    m_layout = new QBoxLayout(direction, this);
    setLayout(m_layout);
    m_layout->setMargin(0);
    m_layout->setSpacing(PREVIEWER_SPACING);

    setLayout(m_layout);
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

void AppPreviewer::updateLayout(QList<WindowPreviewer *> windowPreviewerShow)
{
    if (windowPreviewerShow.isEmpty())
    {
        return;
    }

    Utility::clearLayout(m_layout, false, true);

    // 横竖摆放
    auto direction = getLayoutDirection();
    m_layout->setDirection(direction);
    // 子控件对齐方式：左右、上下
    Qt::AlignmentFlag alignment = getLayoutAlignment();
    m_layout->setAlignment(alignment);

    for (WindowPreviewer *previwer : windowPreviewerShow)
    {
        previwer->show();
        m_layout->addWidget(previwer);
    }

    auto previwer = windowPreviewerShow.first();
    if (QBoxLayout::Direction::LeftToRight == direction)
    {
        setFixedSize((previwer->width() + PREVIEWER_SPACING) * windowPreviewerShow.size() - PREVIEWER_SPACING, previwer->height());
    }
    else
    {
        setFixedSize(previwer->width(), (previwer->height() + PREVIEWER_SPACING) * windowPreviewerShow.size() - PREVIEWER_SPACING);
    }
}

void AppPreviewer::addWindow(QByteArray wmClass, WId wid)
{
    //    KLOG_INFO() << "AppPreviewer::addWindow" << wmClass << wid;
    m_mapWindowPreviewers[wid] = new WindowPreviewer(wid, m_import, this);
    connect(m_mapWindowPreviewers[wid], &WindowPreviewer::closeWindow, this, &AppPreviewer::windowClose);
    connect(m_mapWindowPreviewers[wid], &WindowPreviewer::hideWindow, this, &QWidget::hide, Qt::DirectConnection);

    // 需要显示时才显示
    m_mapWindowPreviewers[wid]->hide();
}

void AppPreviewer::removeWindow(WId wid)
{
    WindowPreviewer *previewr = m_mapWindowPreviewers.take(wid);
    if (previewr)
    {
        delete previewr;
        previewr = nullptr;
    }
}

void AppPreviewer::showPreviewer(WId wid, QWidget *triggerWidget)
{
    // KLOG_INFO() << "AppPreviewer::showPreviewer" << wid;
    QList<WindowPreviewer *> windowPreviewerShow;

    // 根据当前模式，显示不一样的结果
    if (SettingProcess::getValue(TASKBAR_SHOW_APP_BTN_TAIL_KEY).toBool())
    {
        // 显示单个
        if (m_mapWindowPreviewers.contains(wid))
        {
            windowPreviewerShow.push_back(m_mapWindowPreviewers[wid]);
        }
    }
    else
    {
        // 显示一类
        auto iter = m_mapWindowPreviewers.begin();
        while (iter != m_mapWindowPreviewers.end())
        {
            windowPreviewerShow.push_back(iter.value());
            iter++;
        }
    }

    updateLayout(windowPreviewerShow);

    auto oriention = m_import->getPanel()->getOrientation();
    Utility::updatePopWidgetPos(oriention, triggerWidget, this);

    setVisible(true);
}

void AppPreviewer::hidePreviewer(WId wid)
{
    if (!geometry().contains(QCursor::pos()))
    {
        setVisible(false);
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
        setVisible(false);
    }
}

}  // namespace Taskbar

}  // namespace Kiran
