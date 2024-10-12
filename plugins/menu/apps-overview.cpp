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
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#include <ks-i.h>
#include <qt5-log-i.h>
#include <KActivities/ResourceInstance>
#include <KIO/ApplicationLauncherJob>
#include <KSycoca>
#include <QAbstractItemModel>
#include <QAction>
#include <QCursor>
#include <QFileInfo>
#include <QMap>
#include <QMenu>
#include <QProcess>

#include "apps-overview.h"
#include "ui_apps-overview.h"

namespace Kiran
{
namespace Menu
{
AppsOverview::AppsOverview(QWidget *parent)
    : QWidget(parent),
      m_ui(new Ui::AppsOverview)
{
    m_ui->setupUi(this);

    init();
}

AppsOverview::~AppsOverview()
{
    delete m_ui;
}

void AppsOverview::init()
{
    //    m_ui->m_treeWidgetApps->clear();
    //禁用双击展开
    m_ui->m_treeWidgetApps->setExpandsOnDoubleClick(false);
    //隐藏小三角
    m_ui->m_treeWidgetApps->setRootIsDecorated(false);
    //载入应用列表
    loadApps();

    //剔除列表下为空的顶级项
    for (int i = 0; i < m_ui->m_treeWidgetApps->topLevelItemCount(); i++)
    {
        auto item = m_ui->m_treeWidgetApps->topLevelItem(i);
        if (0 == item->childCount())
        {
            delete m_ui->m_treeWidgetApps->takeTopLevelItem(i);
        }
    }

    m_ui->m_lineEditSearch->setPlaceholderText(tr("Search application"));

    QPalette p = m_ui->m_lineEditSearch->palette();

    m_ui->m_lineEditSearch->setPalette(p);

    connect(KSycoca::self(), SIGNAL(databaseChanged()), this, SLOT(updateApp()));
}

void AppsOverview::loadApps()
{
    m_ui->m_treeWidgetApps->clear();

    KServiceGroup::Ptr group = KServiceGroup::root();
    auto groups = group->groupEntries();
    for (auto it = groups.begin(); it != groups.end(); it++)
    {
        KSycocaEntry *p = it->data();
        //        KLOG_INFO() << "KServiceGroup:" << p->name();
        addGroup(p);
    }

    //展开应用列表
    m_ui->m_treeWidgetApps->expandAll();
}

void AppsOverview::addGroup(KSycocaEntry *entry, const QString filter, QTreeWidgetItem *parent)
{
    //动态翻译
    static const QMap<QString, const char *> TR_NOOP_STRING = {
        {"Accessories", QT_TR_NOOP("Accessories")},
        {"Development", QT_TR_NOOP("Development")},
        {"Graphics", QT_TR_NOOP("Graphics")},
        {"Internet", QT_TR_NOOP("Internet")},
        {"Multimedia", QT_TR_NOOP("Multimedia")},
        {"Office", QT_TR_NOOP("Office")},
        {"System Tools", QT_TR_NOOP("System Tools")},
        {"Other", QT_TR_NOOP("Other")}};

    if (!filter.isEmpty())
    {
        return;
    }

    KServiceGroup *g = static_cast<KServiceGroup *>(entry);
    if (g->entries().size() <= 0 || g->noDisplay())
    {
        return;
    }

    KLOG_INFO() << "addGroup:" << g->name() << g->caption() << "parent ptr:" << parent << g->entries().size();

    // 中间层级省略,显示一级目录
    if (!parent)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(m_ui->m_treeWidgetApps);
        item->setIcon(0, QIcon::fromTheme(KS_ICON_MENU_GROUP_SYMBOLIC));
        item->setText(0, TR_NOOP_STRING.contains(g->caption()) ? tr(TR_NOOP_STRING[g->caption()]) : g->caption());
        parent = item;
    }

    KLOG_INFO() << "KServiceGroup:" << g->name() << g->caption() << "group ptr:" << parent;

    KServiceGroup::List list = g->entries();
    for (KServiceGroup::List::ConstIterator it = list.begin(); it != list.end(); it++)
    {
        KSycocaEntry *p = it->data();
        if (p->isType(KST_KServiceGroup))
        {
            addGroup(p, filter, parent);
        }
        else if (p->isType(KST_KService))
        {
            addItem(p, filter, parent);
        }
    }
}

void AppsOverview::addItem(KSycocaEntry *entry, const QString filter, QTreeWidgetItem *parent)
{
    KService *s = static_cast<KService *>(entry);

    // 过滤不需要显示的APP
    if (s->noDisplay())
    {
        return;
    }
    // X-KIRAN-NoDisplayg 是kiran桌面配置的不需要显示的应用
    auto kiranNoDisplay = s->property("X-KIRAN-NoDisplay", QVariant::Bool);
    if (kiranNoDisplay.isValid() && kiranNoDisplay.toBool())
    {
        return;
    }

    if (!filter.isEmpty())
    {
        //TODO: 支持拼音搜索
        if (!entry->name().contains(filter, Qt::CaseInsensitive))
        {
            return;
        }
    }

    // 无图标不显示
    QIcon icon = QIcon::fromTheme(s->icon());
    if (icon.isNull())
    {
        // 支持某些desktop文件不规范的情况，如 icon=xx.png
        icon = QIcon::fromTheme(QFileInfo(s->icon()).baseName(), QIcon::fromTheme(QStringLiteral("unknown")));
    }
    if (icon.isNull())
    {
        return;
    }

    if (!parent)
    {
        parent = m_ui->m_treeWidgetApps->topLevelItem(m_ui->m_treeWidgetApps->topLevelItemCount() - 1);
        if (!parent)
        {
            return;
        }
    }
    QTreeWidgetItem *item = new QTreeWidgetItem(parent);
    item->setIcon(0, icon);
    item->setText(0, s->name());
    item->setData(0, Qt::UserRole, s->storageId());
}

void AppsOverview::updateApp()
{
    KLOG_INFO() << "AppsOverview::updateApp";
}

void AppsOverview::on_m_treeWidgetApps_itemClicked(QTreeWidgetItem *item, int column)
{
    QVariant itemData = item->data(0, Qt::UserRole);
    if (itemData.isValid())
    {
        emit runApp(itemData.toString());
    }
    else
    {
        if (item->isExpanded())
        {
            //折叠
            //            m_ui->m_treeWidgetApps->collapseAll();
            item->setExpanded(false);
        }
        else
        {
            //展开后滚动到最上面
            //            m_ui->m_treeWidgetApps->expandAll();
            item->setExpanded(true);

            m_ui->m_treeWidgetApps->scrollToItem(item, QAbstractItemView::PositionAtTop);
        }
    }
}

void AppsOverview::on_m_treeWidgetApps_itemPressed(QTreeWidgetItem *item, int column)
{
    if (qApp->mouseButtons() == Qt::RightButton)
    {
        QVariant itemData = item->data(0, Qt::UserRole);
        if (!itemData.isValid())
        {
            return;
        }

        QString appId = itemData.toString();

        bool isCheckOK = false;

        QMenu menu;
        menu.addAction(tr("Run app"), this, [=]()
                       {
                           emit runApp(appId);
                       });
        menu.addAction(tr("Add to desktop"), this, [=]()
                       {
                           emit addToDesktop(appId);
                       });

        emit isInFavorite(appId, isCheckOK);
        if (!isCheckOK)
        {
            menu.addAction(tr("Add to favorite"), this, [=]()
                           {
                               emit addToFavorite(appId);
                           });
        }
        else
        {
            menu.addAction(tr("Remove from favorite"), this, [=]()
                           {
                               emit removeFromFavorite(appId);
                           });
        }

        isCheckOK = false;
        KService::Ptr s = KService::serviceByMenuId(appId);
        QUrl url = QUrl::fromLocalFile(s->entryPath());
        emit isInTasklist(url, isCheckOK);
        if (!isCheckOK)
        {
            menu.addAction(tr("Add to tasklist"), this, [=]()
                           {
                               KService::Ptr s = KService::serviceByMenuId(appId);
                               QUrl url = QUrl::fromLocalFile(s->entryPath());
                               emit addToTasklist(url);
                           });
        }
        else
        {
            menu.addAction(tr("Remove from tasklist"), this, [=]()
                           {
                               KService::Ptr s = KService::serviceByMenuId(appId);
                               QUrl url = QUrl::fromLocalFile(s->entryPath());
                               emit removeFromTasklist(url);
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
                                                 auto *job = new KIO::ApplicationLauncherJob(serviceAction);
                                                 job->start();

                                                 //通知kactivitymanagerd
                                                 KActivities::ResourceInstance::notifyAccessed(QUrl(QStringLiteral("applications:") + s->storageId()));
                                             });
            if (serviceAction.isSeparator())
            {
                action->setSeparator(true);
            }
        }

        menu.exec(QCursor::pos());
    }
}

void AppsOverview::on_m_lineEditSearch_textChanged(const QString &arg1)
{
    if (arg1.isEmpty())
    {
        loadApps();
        return;
    }

    m_ui->m_treeWidgetApps->clear();

    QTreeWidgetItem *item = new QTreeWidgetItem(m_ui->m_treeWidgetApps);
    item->setIcon(0, QIcon::fromTheme(KS_ICON_MENU_GROUP_SYMBOLIC));
    //        KLOG_INFO() << "propertyNames:" << g->name() << g->caption() << g->icon() << g->comment() << g->noDisplay() << g->baseGroupName();

    item->setText(0, tr("result"));
    m_ui->m_treeWidgetApps->addTopLevelItem(item);

    KServiceGroup::Ptr group = KServiceGroup::root();
    addGroup(group.data(), arg1, item);

    m_ui->m_treeWidgetApps->expandAll();
}

void AppsOverview::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    m_ui->m_lineEditSearch->setFocus();
}

}  // namespace Menu

}  // namespace Kiran
