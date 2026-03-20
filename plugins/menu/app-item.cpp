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
#include <KActivities/ResourceInstance>
#include <KService/KService>
#include <QDrag>
#include <QIcon>
#include <QKeyEvent>
#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>

#include "app-item.h"
#include "icon-utils.h"
#include "lib/common/app-launcher.h"
#include "lib/common/logging-category.h"

namespace Kiran
{
namespace Menu
{
AppItem::AppItem(QWidget *parent)
    : StyledButton(parent),
      m_appId("")
{
    setFixedSize(90, 90);
    setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    setCheckable(false);
}

AppItem::~AppItem() = default;

void AppItem::setAppId(const QString &appId)
{
    m_appId = appId;
    KService::Ptr s = KService::serviceByMenuId(m_appId);
    if (s)
    {
        QIcon icon = loadAppIcon(s->icon());
        if (icon.isNull())
        {
            icon = QIcon::fromTheme("application-x-executable");
        }
        setIcon(icon);
        setText(s->name());

        QString tooltip = s->name();
        // 当comment与name相同时，不显示comment
        if (!s->comment().isEmpty() && (s->name() != s->comment()))
        {
            tooltip += "\n" + s->comment();
        }

        setToolTip(tooltip);
    }
}

void AppItem::contextMenuEvent(QContextMenuEvent *event)
{
    if (m_appId.isEmpty())
    {
        return;
    }

    QMenu menu;
    bool check_result = false;

    KService::Ptr s = KService::serviceByMenuId(m_appId);
    if (!s)
    {
        return;  // 无效的 m_appId
    }

    menu.addAction(tr("Run app"), this, [=]()
                   {
                       emit runApp(m_appId);
                   });
    menu.addAction(tr("Add to desktop"), this, [=]()
                   {
                       emit addToDesktop(m_appId);
                   });

    emit isInFavorite(m_appId, check_result);
    if (!check_result)
    {
        menu.addAction(tr("Add to favorite"), this, [=]()
                       {
                           emit addToFavorite(m_appId);
                       });
    }
    else
    {
        menu.addAction(tr("Remove from favorite"), this, [=]()
                       {
                           emit removeFromFavorite(m_appId);
                       });
    }

    QUrl url = QUrl::fromLocalFile(s->entryPath());
    emit isInFixedApps(url, check_result);
    if (!check_result)
    {
        menu.addAction(tr("Add to tasklist"), this, [=]()
                       {
                           emit addToFixedApps(url);
                       });
    }
    else
    {
        menu.addAction(tr("Remove from tasklist"), this, [=]()
                       {
                           emit removeFromFixedApps(url);
                       });
    }

    // 自带菜单
    bool firstAdd = true;
    for (const KServiceAction &serviceAction : s->actions())
    {
        if (serviceAction.noDisplay())
        {
            continue;
        }

        if (firstAdd)
        {
            menu.addSeparator();
            firstAdd = false;
        }
        QAction *action = menu.addAction(QIcon::fromTheme(serviceAction.icon()), serviceAction.text(), this, [=]()
                                         {
                                             Common::appLauncher(serviceAction, s->storageId());
                                         });
        if (serviceAction.isSeparator())
        {
            action->setSeparator(true);
        }
    }

    menu.exec(mapToGlobal(event->pos()));

    update();
}

void AppItem::keyPressEvent(QKeyEvent *event)
{
    if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) && !m_appId.isEmpty())
    {
        emit runApp(m_appId);
        event->accept();
        return;
    }
    StyledButton::keyPressEvent(event);
}

void AppItem::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        emit runApp(m_appId);
    }

    StyledButton::mouseReleaseEvent(event);
}

void AppItem::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        m_pressPoint = event->pos();
    }
}

void AppItem::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        if (qAbs(event->pos().x() - m_pressPoint.x()) < 10 && qAbs(event->pos().y() - m_pressPoint.y()) < 10)
        {
            return;
        }

        KService::Ptr s = KService::serviceByMenuId(m_appId);
        if (!s)
        {
            KLOG_WARNING(LCMenu) << "Invalid service for app ID:" << m_appId;
            return;
        }

        // The QDrag must be constructed on the heap with a parent QObject to ensure that Qt can clean up after the drag and drop operation has been completed.
        // The QMimeData and QDrag objects created by the source widget should not be deleted - they will be destroyed by Qt
        auto *drag = new QDrag(this);
        auto *mimeData = new QMimeData;
        QByteArray data = QUrl::fromLocalFile(s->entryPath()).toString().toLocal8Bit();
        mimeData->setData("text/uri-list", data);
        drag->setMimeData(mimeData);
        drag->setPixmap(icon().pixmap(40, 40));
        drag->exec(Qt::CopyAction);
    }
}
}  // namespace Menu
}  // namespace Kiran
