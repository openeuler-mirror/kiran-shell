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

#include <kiran-style/style-palette.h>
#include <qt5-log-i.h>
#include <KService/KService>
#include <QDrag>
#include <QIcon>
#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>

#include "app-item.h"

AppItem::AppItem(QWidget *parent)
    : StyledButton(parent)
{
    setFixedSize(90, 90);
    setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    setCheckable(false);
}

AppItem::~AppItem()
{
}

void AppItem::setAppId(QString id)
{
    m_appId = id;
    KService::Ptr s = KService::serviceByMenuId(m_appId);
    if (s)
    {
        setIcon(QIcon::fromTheme(s->icon()));
        setText(s->name());
        setToolTip(s->comment());
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

    menu.addAction(tr("Run app"), this, [=]()
                   { emit runApp(m_appId); });
    menu.addAction(tr("Add to desktop"), this, [=]()
                   { emit addToDesktop(m_appId); });

    emit isInFavorite(m_appId, check_result);
    if (!check_result)
    {
        menu.addAction(tr("Add to favorite"), this, [=]()
                       { emit addToFavorite(m_appId); });
    }
    else
    {
        menu.addAction(tr("Remove from favorite"), this, [=]()
                       { emit removeFromFavorite(m_appId); });
    }

    check_result = false;
    KService::Ptr s = KService::serviceByMenuId(m_appId);
    QUrl url = QUrl::fromLocalFile(s->entryPath());
    emit isInTasklist(url, check_result);
    if (!check_result)
    {
        menu.addAction(tr("Add to tasklist"), this, [=]()
                       {
                           KService::Ptr s = KService::serviceByMenuId(m_appId);
                           QUrl url = QUrl::fromLocalFile(s->entryPath());
                           emit addToTasklist(url);
                       });
    }
    else
    {
        menu.addAction(tr("Remove from tasklist"), this, [=]()
                       {
                           KService::Ptr s = KService::serviceByMenuId(m_appId);
                           QUrl url = QUrl::fromLocalFile(s->entryPath());
                           emit removeFromTasklist(url);
                       });
    }

    menu.exec(mapToGlobal(event->pos()));

    update();
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

        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;
        KService::Ptr s = KService::serviceByMenuId(m_appId);
        QByteArray data = QUrl::fromLocalFile(s->entryPath()).toString().toLocal8Bit();
        mimeData->setData("text/uri-list", data);
        drag->setMimeData(mimeData);
        drag->setPixmap(icon().pixmap(40, 40));
        drag->exec(Qt::MoveAction);
    }
}
