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

#include <KService/KService>
#include <QIcon>
#include <QMenu>
#include <QMouseEvent>
#include <QProcess>

#include "app-item.h"
#include "lib/common/common.h"
#include "ui_app-item.h"

AppItem::AppItem(QWidget *parent)
    : KiranColorBlock(parent),
      m_ui(new Ui::AppItem)
{
    m_ui->setupUi(this);
    m_ui->m_labelName->clear();
}

AppItem::~AppItem()
{
    delete m_ui;
}

void AppItem::setAppId(QString id)
{
    m_appId = id;
    KService::Ptr s = KService::serviceByMenuId(m_appId);
    if (s)
    {
        //长度不够，显示省略号
        //m_labelName不是固定大小，所以使用其他控件属性值计算其长度
        QFontMetrics fontMetrics = m_ui->m_labelName->fontMetrics();
        auto margins = m_ui->m_gridLayoutAppItem->contentsMargins();
        int elidedTextLen = width() - margins.left() - margins.right();
        QString elideText = getElidedText(fontMetrics, s->name(), elidedTextLen);
        m_ui->m_labelName->setText(elideText);

        auto pix = QIcon::fromTheme(s->icon()).pixmap(56, 56);
        m_ui->m_labelIcon->setPixmap(pix);

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
    emit isInTasklist(m_appId, check_result);
    if (!check_result)
    {
        menu.addAction(tr("Add to tasklist"), this, [=]()
                       { emit addToTasklist(m_appId); });
    }
    else
    {
        menu.addAction(tr("Remove from tasklist"), this, [=]()
                       { emit removeFromTasklist(m_appId); });
    }

    menu.exec(mapToGlobal(event->pos()));
}

void AppItem::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        emit runApp(m_appId);

        return;
    }

    QWidget::mouseReleaseEvent(event);
}
