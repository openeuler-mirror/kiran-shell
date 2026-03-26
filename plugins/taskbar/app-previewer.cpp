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

enum
{
    PREVIEWER_SPACING = 3
};

namespace Kiran
{
namespace Taskbar
{
// See also: 
// Kwin AbstractClient::belongsToLayer
// Kwin显示桌面时，将Desktop窗口提前至AboveLayer层，导致预览窗口被遮挡。
// 参考plasma，将预览窗口层级提前至ToolTip层。
AppPreviewer::AppPreviewer(IAppletImport *import, QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint | Qt::ToolTip),
      m_import(import)
{
    auto *window = (Window *)parent;
    auto *panelObject = dynamic_cast<QObject *>(m_import->getPanel());

    // 连接预览窗口相关信号
    connect(window, &Window::windowChanged, this, &AppPreviewer::windowChanged);
    connect(window, &Window::activeWindowChanged, this, &AppPreviewer::activeWindowChanged);
    connect(window, &Window::previewerShow, this, &AppPreviewer::showPreviewer);
    connect(window, &Window::previewerHide, this, &AppPreviewer::hidePreviewer);
    connect(window, &Window::previewerShowChange, this, &AppPreviewer::previewerShowChange);

    // 监听panel尺寸/方向变化，同步更新预览窗口
    connect(panelObject, SIGNAL(panelProfileChanged()), this, SLOT(panelProfileChanged()));

    m_hideTimer = new QTimer(this);
    m_hideTimer->setSingleShot(true);
    m_hideTimer->setInterval(500);
    connect(m_hideTimer, &QTimer::timeout, this, &AppPreviewer::hideTimeout);

    // 横竖摆放
    auto direction = getLayoutDirection();
    m_layout = new QBoxLayout(direction, this);
    m_layout->setMargin(0);
    m_layout->setSpacing(PREVIEWER_SPACING);
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

    auto *previwer = windowPreviewerShow.first();
    if (QBoxLayout::Direction::LeftToRight == direction)
    {
        setFixedSize(((previwer->width() + PREVIEWER_SPACING) * windowPreviewerShow.size()) - PREVIEWER_SPACING, previwer->height());
    }
    else
    {
        setFixedSize(previwer->width(), ((previwer->height() + PREVIEWER_SPACING) * windowPreviewerShow.size()) - PREVIEWER_SPACING);
    }
}

void AppPreviewer::addWindow(WId wid)
{
    // 若已存在该窗口的预览（如重复事件），先移除再创建，避免泄漏
    if (m_mapWindowPreviewers.contains(wid))
    {
        removeWindow(wid);
    }

    m_mapWindowPreviewers[wid] = new WindowPreviewer(wid, m_import, this);
    connect(m_mapWindowPreviewers[wid], &WindowPreviewer::hideWindow, [this]()
            {
                setVisible(false);
            });

    // 需要显示时才显示
    m_mapWindowPreviewers[wid]->setVisible(false);
}

void AppPreviewer::removeWindow(WId wid)
{
    auto *previewr = m_mapWindowPreviewers.take(wid);
    if (previewr)
    {
        delete previewr;
        previewr = nullptr;
    }
}

void AppPreviewer::showPreviewer(const QList<WId> &wids, QWidget *triggerWidget)
{
    m_widsCurrentShow = wids;
    m_triggerWidget = triggerWidget;

    m_hideTimer->stop();

    QList<WindowPreviewer *> windowPreviewerShow;

    for (auto wid : wids)
    {
        // 只显示当前桌面的窗口
        if (m_mapWindowPreviewers.contains(wid) && WindowInfoHelper::isOnCurrentDesktop(wid))
        {
            windowPreviewerShow.push_back(m_mapWindowPreviewers[wid]);
        }
    }

    if (windowPreviewerShow.isEmpty())
    {
        return;
    }

    updateLayout(windowPreviewerShow);

    setVisible(true);

    auto oriention = m_import->getPanel()->getOrientation();
    auto *screen = m_import->getPanel()->getScreen();
    Utility::updatePopWidgetPos(screen, oriention, triggerWidget, this);
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

void AppPreviewer::previewerShowChange(const QList<WId> &wids, QWidget *triggerWidget)
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

// panel尺寸或方向变化时的响应：更新所有预览窗口大小，如当前正显示则重新布局和定位
void AppPreviewer::panelProfileChanged()
{
    // 更新所有已存在预览窗口的大小
    for (auto *previewer : m_mapWindowPreviewers)
    {
        previewer->updateLayout();
    }

    // 若预览窗口当前未显示，无需后续处理
    if (!isVisible() || m_widsCurrentShow.isEmpty() || !m_triggerWidget)
    {
        return;
    }

    // 重新过滤当前应显示的窗口（可能因桌面切换或窗口关闭而变化）
    QList<WindowPreviewer *> windowPreviewerShow;
    for (auto wid : m_widsCurrentShow)
    {
        if (m_mapWindowPreviewers.contains(wid) && WindowInfoHelper::isOnCurrentDesktop(wid))
        {
            windowPreviewerShow.push_back(m_mapWindowPreviewers[wid]);
        }
    }

    // 无可见窗口则隐藏预览容器
    if (windowPreviewerShow.isEmpty())
    {
        setVisible(false);
        return;
    }

    // 重新布局并调整大小
    updateLayout(windowPreviewerShow);

    // 根据新panel位置重新定位预览窗口
    auto oriention = m_import->getPanel()->getOrientation();
    auto *screen = m_import->getPanel()->getScreen();
    Utility::updatePopWidgetPos(screen, oriention, m_triggerWidget, this);
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

void AppPreviewer::showEvent(QShowEvent *event)
{
    // 任务栏不显示
    KWindowSystem::setState(winId(), NET::SkipTaskbar | NET::SkipPager | NET::SkipSwitcher);
    QWidget::showEvent(event);
}

}  // namespace Taskbar

}  // namespace Kiran
