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
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#include <ks-i.h>
#include <qt5-log-i.h>
#include <KActivities/ResourceInstance>
#include <QAction>
#include <QCursor>
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
    m_ui->m_treeWidgetShowApps->clear();
    //禁用双击展开
    m_ui->m_treeWidgetShowApps->setExpandsOnDoubleClick(false);
    //隐藏小三角
    m_ui->m_treeWidgetShowApps->setRootIsDecorated(false);

    //载入应用列表
    loadApps();

    //剔除列表下为空的顶级项
    for (int i = 0; i < m_ui->m_treeWidgetShowApps->topLevelItemCount(); i++)
    {
        auto item = m_ui->m_treeWidgetShowApps->topLevelItem(i);
        if (0 == item->childCount())
        {
            delete m_ui->m_treeWidgetShowApps->takeTopLevelItem(i);
        }
    }

    m_ui->m_lineEditSearch->setPlaceholderText(tr("Search application"));
}

void AppsOverview::loadApps()
{
    m_ui->m_treeWidgetShowApps->clear();

    KServiceGroup::Ptr group = KServiceGroup::root();
    recursiveService(group.data());

    //展开应用列表
    m_ui->m_treeWidgetShowApps->expandAll();
}

void AppsOverview::recursiveService(KServiceGroup *serviceGroup, const QString &filter, QTreeWidgetItem *parent)
{
    KServiceGroup::List list = serviceGroup->entries();

    //    KLOG_INFO() << "entry number: " << list.size() << "group: " << serviceGroup->name();

    // Iterate over all entries in the group
    for (KServiceGroup::List::ConstIterator it = list.begin(); it != list.end(); it++)
    {
        KSycocaEntry *p = it->data();
        //        KLOG_INFO() << "type: " << p->sycocaType();
        addItem(p, filter, parent);

        if (p->isType(KST_KServiceGroup))
        {
            KServiceGroup *g = static_cast<KServiceGroup *>(p);
            recursiveService(g, filter, parent);
        }
        else if (p->isType(KST_KServiceSeparator))
        {
        }
    }
}

void AppsOverview::addItem(KSycocaEntry *entry, const QString filter, QTreeWidgetItem *parent)
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

    if (entry->isType(KST_KServiceGroup))
    {
        if (!filter.isEmpty())
        {
            return;
        }

        KServiceGroup *g = static_cast<KServiceGroup *>(entry);
        if (g->entries().size() <= 0 || g->noDisplay())
        {
            return;
        }

        QTreeWidgetItem *item = new QTreeWidgetItem(m_ui->m_treeWidgetShowApps);
        item->setIcon(0, QIcon::fromTheme(KS_ICON_MENU_GROUP_SYMBOLIC));
        //        KLOG_INFO() << "propertyNames:" << g->name() << g->caption() << g->icon() << g->comment() << g->noDisplay() << g->baseGroupName();

        item->setText(0, TR_NOOP_STRING.contains(g->caption()) ? tr(TR_NOOP_STRING[g->caption()]) : g->caption());
        m_ui->m_treeWidgetShowApps->addTopLevelItem(item);
    }
    else if (entry->isType(KST_KService))
    {
        KService *service = static_cast<KService *>(entry);

        // 过滤不需要显示的APP
        if (service->noDisplay())
        {
            return;
        }
        // X-KIRAN-NoDisplay 是kiran桌面配置的不需要显示的应用
        auto kiranNoDisplay = service->property("X-KIRAN-NoDisplay", QVariant::Bool);
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

        if (!parent)
        {
            parent = m_ui->m_treeWidgetShowApps->topLevelItem(m_ui->m_treeWidgetShowApps->topLevelItemCount() - 1);
            if (!parent)
            {
                return;
            }
        }
        QTreeWidgetItem *item = new QTreeWidgetItem(parent);
        item->setIcon(0, QIcon::fromTheme(service->icon()));
        item->setText(0, service->name());
        item->setData(0, Qt::UserRole, service->storageId());
    }
}

void AppsOverview::on_m_treeWidgetShowApps_itemClicked(QTreeWidgetItem *item, int column)
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
            m_ui->m_treeWidgetShowApps->collapseAll();
        }
        else
        {
            //展开后滚动到最上面
            m_ui->m_treeWidgetShowApps->expandAll();
            m_ui->m_treeWidgetShowApps->scrollToItem(item, QAbstractItemView::PositionAtTop);
        }
    }
}

void AppsOverview::on_m_treeWidgetShowApps_itemPressed(QTreeWidgetItem *item, int column)
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
                       { emit runApp(appId); });
        menu.addAction(tr("Add to desktop"), this, [=]()
                       { emit addToDesktop(appId); });

        emit isInFavorite(appId, isCheckOK);
        if (!isCheckOK)
        {
            menu.addAction(tr("Add to favorite"), this, [=]()
                           { emit addToFavorite(appId); });
        }
        else
        {
            menu.addAction(tr("Remove from favorite"), this, [=]()
                           { emit removeFromFavorite(appId); });
        }

        isCheckOK = false;
        emit isInTasklist(appId, isCheckOK);
        if (!isCheckOK)
        {
            menu.addAction(tr("Add to tasklist"), this, [=]()
                           { emit addToTasklist(appId); });
        }
        else
        {
            menu.addAction(tr("Remove from tasklist"), this, [=]()
                           { emit removeFromTasklist(appId); });
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

    m_ui->m_treeWidgetShowApps->clear();

    QTreeWidgetItem *item = new QTreeWidgetItem(m_ui->m_treeWidgetShowApps);
    item->setIcon(0, QIcon::fromTheme(KS_ICON_MENU_GROUP_SYMBOLIC));
    //        KLOG_INFO() << "propertyNames:" << g->name() << g->caption() << g->icon() << g->comment() << g->noDisplay() << g->baseGroupName();

    item->setText(0, tr("result"));
    m_ui->m_treeWidgetShowApps->addTopLevelItem(item);

    KServiceGroup::Ptr group = KServiceGroup::root();
    recursiveService(group.data(), arg1, item);

    m_ui->m_treeWidgetShowApps->expandAll();
}

}  // namespace Menu

}  // namespace Kiran
