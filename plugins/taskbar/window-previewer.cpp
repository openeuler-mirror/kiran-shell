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

#include <plugin-i.h>
#include <qt5-log-i.h>
#include <KWindowSystem>
#include <QMenu>
#include <QMouseEvent>
#include <QTimer>

#include "app-previewer.h"
#include "lib/common/utility.h"
#include "lib/common/window-info-helper.h"
#include "window-previewer.h"

namespace Kiran
{
namespace Taskbar
{
WindowPreviewer::WindowPreviewer(WId wid, IAppletImport *import, AppPreviewer *parent)
    : WindowThumbnail(wid, parent),
      m_import(import)
{
    int panelSize = m_import->getPanel()->getSize();
    setFixedSize(panelSize * 4, panelSize * 4);

    m_menu = new QMenu(this);
    // 菜单弹出时，点击其地方，隐藏预览窗口
    connect(m_menu, &QMenu::aboutToHide, this, &WindowPreviewer::hideWindow);
    connect(parent, &AppPreviewer::activeWindowChanged, this, &WindowPreviewer::changedActiveWindow);
}

WindowPreviewer::~WindowPreviewer()
{
}

bool WindowPreviewer::checkCanHide()
{
    return !m_menu->isVisible();
}

void WindowPreviewer::changedActiveWindow(WId wid)
{
    m_widLastActive = wid;
}

void WindowPreviewer::on_m_btnClose_clicked()
{
    emit closeWindow(m_wid);
}
void WindowPreviewer::mouseReleaseEvent(QMouseEvent *event)
{
    if (Qt::LeftButton == event->button())
    {
        emit hideWindow();

        if (m_widLastActive != m_wid)
        {
            WindowInfoHelper::activateWindow(m_wid);
        }
        else
        {
            WindowInfoHelper::minimizeWindow(m_wid);
            m_widLastActive = 0;
        }
    }
}

void WindowPreviewer::contextMenuEvent(QContextMenuEvent *event)
{
    m_menu->clear();

    m_menu->addAction(tr("Close window"), this, [=]()
                      {
                          emit closeWindow(m_wid);
                      });

    if (WindowInfoHelper::isMaximized(m_wid))
    {
        m_menu->addAction(tr("Restore"), this, [=]()
                          {
                              WindowInfoHelper::maximizeWindow(m_wid, false);
                          });
    }
    else
    {
        m_menu->addAction(tr("Maximize"), this, [=]()
                          {
                              WindowInfoHelper::maximizeWindow(m_wid, true);
                          });
    }

    if (!WindowInfoHelper::isMinimized(m_wid))
    {
        m_menu->addAction(tr("Minimize"), this, [=]()
                          {
                              WindowInfoHelper::minimizeWindow(m_wid);
                          });
    }

    if (WindowInfoHelper::isKeepAboved(m_wid))
    {
        m_menu->addAction(tr("Do not keep above"), this, [=]()
                          {
                              WindowInfoHelper::setKeepAbove(m_wid, false);
                          });
    }
    else
    {
        m_menu->addAction(tr("Keep above"), this, [=]()
                          {
                              WindowInfoHelper::setKeepAbove(m_wid, true);
                          });
    }

    m_menu->exec(mapToGlobal(event->pos()));
}

}  // namespace Taskbar

}  // namespace Kiran
