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

#include <qt5-log-i.h>
#include <KWindowSystem/NETWM>
#include <QBoxLayout>
#include <QTimer>

#include "app-previewer.h"
#include "ks-i.h"
#include "lib/common/utility.h"
#include "lib/common/window-info-helper.h"
#include "plugin-i.h"
#include "window.h"

#define PREVIEWER_SPACING 3

namespace Kiran
{
namespace Taskbar
{
AppPreviewer::AppPreviewer(IAppletImport *import, QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint | Qt::Tool),
      m_import(import)
{
    Window *window = (Window *)parent;
    connect(window, &Window::windowAdded, this, &AppPreviewer::addWindow);
    connect(window, &Window::windowRemoved, this, &AppPreviewer::removeWindow);
    connect(window, &Window::windowChanged, this, &AppPreviewer::windowChanged);
    connect(window, &Window::activeWindowChanged, this, &AppPreviewer::activeWindowChanged);
    connect(window, &Window::previewerShow, this, &AppPreviewer::showPreviewer);
    connect(window, &Window::previewerHide, this, &AppPreviewer::hidePreviewer);
    connect(window, &Window::previewerShowChange, this, &AppPreviewer::previewerShowChange);

    m_hideTimer = new QTimer(this);
    m_hideTimer->setSingleShot(true);
    m_hideTimer->setInterval(500);
    connect(m_hideTimer, &QTimer::timeout, this, &AppPreviewer::hideTimeout);

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
    connect(m_mapWindowPreviewers[wid], &WindowPreviewer::closeWindow, [this](WId wid)
            {
                // 关闭窗口
                WindowInfoHelper::closeWindow(wid);
                setVisible(false);
            });
    connect(m_mapWindowPreviewers[wid], &WindowPreviewer::hideWindow, [this]()
            {
                setVisible(false);
            });

    // 需要显示时才显示
    m_mapWindowPreviewers[wid]->setVisible(false);
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

void AppPreviewer::showPreviewer(QList<WId> wids, QWidget *triggerWidget)
{
    m_hideTimer->stop();

    //    KLOG_INFO() << "AppPreviewer::showPreviewer" << wids;
    QList<WindowPreviewer *> windowPreviewerShow;

    for (auto wid : wids)
    {
        if (m_mapWindowPreviewers.contains(wid))
        {
            windowPreviewerShow.push_back(m_mapWindowPreviewers[wid]);
        }
    }

    updateLayout(windowPreviewerShow);

    setVisible(true);

    auto oriention = m_import->getPanel()->getOrientation();
    Utility::updatePopWidgetPos(oriention, triggerWidget, this);
}

void AppPreviewer::hidePreviewer()
{
    m_hideTimer->start();
}

void AppPreviewer::hideTimeout()
{
    if (!geometry().contains(QCursor::pos()))
    {
        setVisible(false);
    }
}

void AppPreviewer::previewerShowChange(QList<WId> wids, QWidget *triggerWidget)
{
    if (isVisible())
    {
        setVisible(false);
    }
    else
    {
        showPreviewer(wids, triggerWidget);
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
        m_hideTimer->start();
    }
}

}  // namespace Taskbar

}  // namespace Kiran
