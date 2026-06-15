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
#include <QMenu>
#include <QMouseEvent>

#include "app-previewer.h"
#include "lib/common/utility.h"
#include "lib/common/window-manager.h"
#include "plugin-i.h"
#include "window-previewer.h"

namespace Kiran
{
namespace Taskbar
{
WindowPreviewer::WindowPreviewer(WId wid, IAppletImport *import, AppPreviewer *parent)
    : WindowThumbnail(wid, parent),
      m_import(import)
{
    updateLayout();

    m_menu = new QMenu(this);
    // 菜单弹出时，点击其地方，隐藏预览窗口
    connect(m_menu, &QMenu::aboutToHide, this, &WindowPreviewer::hideWindow);
    connect(parent, &AppPreviewer::activeWindowChanged, this, &WindowPreviewer::changedActiveWindow);
}

WindowPreviewer::~WindowPreviewer() = default;

// 根据当前panel尺寸更新预览窗口大小（尺寸为panel尺寸的4倍）
void WindowPreviewer::updateLayout()
{
    int panelSize = m_import->getPanel()->getSize();
    setFixedSize(panelSize * 4, panelSize * 4);
}

bool WindowPreviewer::checkCanHide()
{
    return !m_menu->isVisible();
}

void WindowPreviewer::changedActiveWindow(WId wid)
{
    m_widLastActive = wid;
}

void WindowPreviewer::on_btnClose_clicked()
{
    WindowManagerInstance.closeWindow(m_wid);
    setVisible(false);
}
void WindowPreviewer::mouseReleaseEvent(QMouseEvent *event)
{
    if (Qt::LeftButton == event->button())
    {
        // 保持与kiran-menu一致，点击状态栏预览图之后不隐藏状态栏预览图
        // emit hideWindow();

        if (m_widLastActive != m_wid)
        {
            WindowManagerInstance.activateWindow(m_wid);
        }
        else
        {
            WindowManagerInstance.minimizeWindow(m_wid);
            m_widLastActive = 0;
        }
    }
}

void WindowPreviewer::contextMenuEvent(QContextMenuEvent *event)
{
    m_menu->clear();

    m_menu->addAction(tr("Close window"), this, [=]()
                      {
                          on_btnClose_clicked();
                      });

    if (WindowManagerInstance.isMaximized(m_wid))
    {
        m_menu->addAction(tr("Restore"), this, [=]()
                          {
                              WindowManagerInstance.maximizeWindow(m_wid, false);
                          });
    }
    else
    {
        m_menu->addAction(tr("Maximize"), this, [=]()
                          {
                              WindowManagerInstance.maximizeWindow(m_wid, true);
                          });
    }

    if (!WindowManagerInstance.isMinimized(m_wid))
    {
        m_menu->addAction(tr("Minimize"), this, [=]()
                          {
                              WindowManagerInstance.minimizeWindow(m_wid);
                          });
    }

    if (WindowManagerInstance.isKeepAbove(m_wid))
    {
        m_menu->addAction(tr("Do not keep above"), this, [=]()
                          {
                              WindowManagerInstance.setKeepAbove(m_wid, false);
                          });
    }
    else
    {
        m_menu->addAction(tr("Keep above"), this, [=]()
                          {
                              WindowManagerInstance.setKeepAbove(m_wid, true);
                          });
    }

    // 获取当前有多少个桌面
    int desktopCount = WindowManagerInstance.numberOfDesktops();
    if (desktopCount > 1)
    {
        auto *menuDesktop = m_menu->addMenu(tr("Move to other desktop"));
        for (int i = 1; i <= desktopCount; i++)
        {
            auto action = menuDesktop->addAction(tr("workspace") + QString::number(i), this, [this, i]()
                                                 {
                                                      WindowManagerInstance.moveWindowToDesktop(m_wid, i);
                                                  });
            if (WindowManagerInstance.getDesktopOfWindow(m_wid) == i)
            {
                action->setEnabled(false);
            }
        }
    }

    if (!WindowManagerInstance.isMinimized(m_wid))
    {
        m_menu->addAction(tr("move"), this, [=]()
                          {
                              WindowManagerInstance.moveResizeWindow(m_wid);
                          });
    }

    m_menu->exec(mapToGlobal(event->pos()));
}

}  // namespace Taskbar

}  // namespace Kiran
