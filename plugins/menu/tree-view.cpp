/**
 * Copyright (c) 2020 ~ 2025 KylinSec Co., Ltd.
 * kiran-shell is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     liuxinhao <liuxinhao@kylinsec.com.cn>
 */
#include <KService>
#include <QDir>
#include <QDrag>
#include <QFileInfo>
#include <QKeyEvent>
#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>

#include "apps-model.h"
#include "lib/common/app-launcher.h"
#include "lib/common/logging-category.h"
#include "lib/common/utility.h"
#include "recent-files-model.h"
#include "tree-delegate.h"
#include "tree-view.h"

namespace Kiran
{
namespace Menu
{
TreeView::TreeView(QWidget *parent)
    : QTreeView(parent)
{
    // 样式代理
    auto *itemDelegate = new TreeDelegate(this);
    setItemDelegate(itemDelegate);
    setIndentation(0);  // 为了绘制底色时，区域为完整一行，设置缩进为0，在绘制中加入缩进
    setMouseTracking(true);
    //    setAttribute(Qt::WA_TranslucentBackground);
    QPalette palette = this->palette();
    palette.setBrush(QPalette::Base, QBrush(QColor(0, 0, 0, 0)));
    setPalette(palette);

    // 隐藏根节点
    setRootIsDecorated(false);
    setHeaderHidden(true);

    // 按像素滚动，隐藏最下方空白
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
}

TreeView::~TreeView()
{
}

AppsView::AppsView(QWidget *parent)
    : TreeView(parent)
{
    m_model = new AppsModel(this);
    setModel(m_model);

    connect(this, &QAbstractItemView::clicked, this, [this](const QModelIndex &index)
            {
                if (index.isValid())
                {
                    if (m_model->isCategory(index))
                    {
                        setExpanded(index, !isExpanded(index));
                    }
                    else
                    {
                        m_model->getID(index);
                        emit runApp(m_model->getID(index));
                    }
                }
            });

    connect(m_model, &AppsModel::modelReset, this, [this]()
            {
                expandAll();
            });
}

AppsView::~AppsView()
{
}

void AppsView::setFilterText(const QString &text)
{
    m_model->setFilterText(text);
    expandAll();
}

void AppsView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return)
    {
        QModelIndex current = this->currentIndex();
        if (!current.isValid())
        {
            return;
        }

        if (m_model->isCategory(current))
        {
            setExpanded(current, !isExpanded(current));
        }
        else
        {
            m_model->getID(current);
            emit runApp(m_model->getID(current));
        }
    }
    TreeView::keyPressEvent(event);
}

void AppsView::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        m_pressPoint = event->pos();
    }
    TreeView::mousePressEvent(event);
}

void AppsView::mouseMoveEvent(QMouseEvent *event)
{
    do
    {
        if (!(event->buttons() & Qt::LeftButton))
        {
            break;
        }

        if (qAbs(event->pos().x() - m_pressPoint.x()) < 10 && qAbs(event->pos().y() - m_pressPoint.y()) < 10)
        {
            break;
        }

        const auto &index = indexAt(event->pos());
        if (!index.isValid())
        {
            break;
        }

        if (m_model->isCategory(index))
        {
            break;
        }

        const auto &appStoragedID = m_model->getID(index);
        if (appStoragedID.isEmpty())
        {
            break;
        }

        auto appIcon = m_model->getIcon(index);
        auto *drag = new QDrag(this);
        auto *mimeData = new QMimeData;
        KService::Ptr s = KService::serviceByMenuId(appStoragedID);
        QByteArray data = QUrl::fromLocalFile(s->entryPath()).toString().toLocal8Bit();
        mimeData->setData("text/uri-list", data);
        drag->setMimeData(mimeData);
        drag->setPixmap(appIcon.pixmap(40, 40));
        drag->exec(Qt::CopyAction);
    } while (0);
    TreeView::mouseMoveEvent(event);
}

void AppsView::contextMenuEvent(QContextMenuEvent *event)
{
    const auto &index = indexAt(event->pos());
    if (!index.isValid())
    {
        return;
    }

    if (m_model->isCategory(index))
    {
        setExpanded(index, !isExpanded(index));
        return;
    }

    auto appStoragedID = m_model->getID(index);

    QMenu menu(this);
    menu.addAction(tr("Run app"), this, [=]()
                   {
                       emit runApp(appStoragedID);
                   });
    menu.addAction(tr("Add to desktop"), this, [=]()
                   {
                       emit addToDesktop(appStoragedID);
                   });

    // 收藏夹
    bool isCheckOK;
    emit isInFavorite(appStoragedID, isCheckOK);
    if (!isCheckOK)
    {
        menu.addAction(tr("Add to favorite"), this, [=]()
                       {
                           emit addToFavorite(appStoragedID);
                       });
    }
    else
    {
        menu.addAction(tr("Remove from favorite"), this, [=]()
                       {
                           emit removeFromFavorite(appStoragedID);
                       });
    }

    // 任务栏选项
    emit isInFixedApps(appStoragedID, isCheckOK);
    KService::Ptr s = KService::serviceByMenuId(appStoragedID);
    QUrl url = QUrl::fromLocalFile(s->entryPath());
    emit isInFixedApps(url, isCheckOK);
    if (!isCheckOK)
    {
        menu.addAction(tr("Add to tasklist"), this, [=]()
                       {
                           KService::Ptr s = KService::serviceByMenuId(appStoragedID);
                           QUrl url = QUrl::fromLocalFile(s->entryPath());
                           emit addToFixedApps(url);
                       });
    }
    else
    {
        menu.addAction(tr("Remove from tasklist"), this, [=]()
                       {
                           KService::Ptr s = KService::serviceByMenuId(appStoragedID);
                           QUrl url = QUrl::fromLocalFile(s->entryPath());
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

    menu.exec(event->globalPos());
}

RecentFilesView::RecentFilesView(QWidget *parent)
    : TreeView(parent)
{
    m_model = new RecentFilesModel(this);
    setModel(m_model);
}

RecentFilesView::~RecentFilesView()
{
}

void RecentFilesView::setFilterText(const QString &text)
{
    m_model->setFilterText(text);
}

void RecentFilesView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return)
    {
        auto currentIdx = this->currentIndex();
        if (currentIdx.isValid())
        {
            emit fileItemClicked(m_model->getFilePath(currentIdx));
            event->accept();
            return;
        }
    }

    TreeView::keyPressEvent(event);
}

void RecentFilesView::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        const auto &pos = event->pos();
        QModelIndex index = indexAt(pos);
        if (index.isValid())
        {
            const auto &filePath = m_model->getFilePath(index);
            emit fileItemClicked(filePath);
            event->accept();
            return;
        }
    }

    TreeView::mousePressEvent(event);
}

void RecentFilesView::contextMenuEvent(QContextMenuEvent *event)
{
    const auto &index = indexAt(event->pos());
    if (!index.isValid())
    {
        return;
    }

    auto filePath = m_model->getFilePath(index);

    QMenu menu(this);
    menu.addAction(tr("Open"), this, [=]()
                   {
                       emit fileItemClicked(filePath);
                   });
    menu.addAction(tr("Open file directory"), this, [=]()
                   {
                       emit fileItemClicked(QFileInfo(filePath).dir().path());
                   });
    menu.addAction(tr("Remove"), this, [=]()
                   {
                       m_model->removeFile(filePath);
                   });
    menu.addAction(tr("Remove all"), this, [=]()
                   {
                       m_model->removeAll();
                   });
    menu.exec(event->globalPos());
}

}  // namespace Menu
}  // namespace Kiran
