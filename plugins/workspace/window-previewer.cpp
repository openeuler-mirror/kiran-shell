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
#include <QDrag>
#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>

#include "lib/common/desktop-helper.h"
#include "lib/common/window-info-helper.h"
#include "window-previewer.h"

namespace Kiran
{
namespace Workspace
{
WindowPreviewer::WindowPreviewer(WId wid, QWidget *parent)
    : WindowThumbnail(wid, parent)
{
}

WindowPreviewer::~WindowPreviewer() = default;

void WindowPreviewer::contextMenuEvent(QContextMenuEvent *event)
{
    // 获取当前有多少个桌面
    int desktopCount = DesktopHelper::numberOfDesktops();
    if (desktopCount > 1)
    {
        QMenu menu(this);
        auto *menuDesktop = menu.addMenu(tr("Move to other desktop"));
        for (int i = 1; i <= desktopCount; i++)
        {
            auto *action = menuDesktop->addAction(tr("workspace") + QString::number(i), this, [this, i]()
                                                  {
                                                      DesktopHelper::moveToDesktop(m_wid, i);
                                                  });
            if (WindowInfoHelper::getDesktopOfWindow(m_wid) == i)
            {
                action->setEnabled(false);
            }
        }
        menu.exec(mapToGlobal(event->pos()));
    }
}

void WindowPreviewer::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        m_pressPoint = event->pos();
    }
}

void WindowPreviewer::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        if (qAbs(event->pos().x() - m_pressPoint.x()) < 10 && qAbs(event->pos().y() - m_pressPoint.y()) < 10)
        {
            return;
        }

        // The QDrag must be constructed on the heap with a parent QObject to ensure that Qt can clean up after the drag and drop operation has been completed.
        // The QMimeData and QDrag objects created by the source widget should not be deleted - they will be destroyed by Qt
        auto *drag = new QDrag(this);
        auto *mimeData = new QMimeData;
        QByteArray data = QByteArray::number(m_wid);
        mimeData->setData("wid", data);
        drag->setMimeData(mimeData);
        drag->setPixmap(grab().scaled(40, 40, Qt::KeepAspectRatio));
        drag->exec(Qt::CopyAction);
    }
}

}  // namespace Workspace
}  // namespace Kiran
