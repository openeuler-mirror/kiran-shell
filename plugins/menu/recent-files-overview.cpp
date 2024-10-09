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

#include <qt5-log-i.h>
#include <QFile>
#include <QFileIconProvider>

#include "recent-files-overview.h"
#include "ui_recent-files-overview.h"

namespace KAStats = KActivities::Stats;
using namespace KAStats;
using namespace KAStats::Terms;

namespace Kiran
{
namespace Menu
{
RecentFilesOverview::RecentFilesOverview(QWidget *parent)
    : QWidget(parent),
      m_ui(new Ui::RecentFilesOverview),
      m_actStatsWatcher(nullptr)
{
    m_ui->setupUi(this);
    init();
}

RecentFilesOverview::~RecentFilesOverview()
{
    delete m_ui;
}

void RecentFilesOverview::on_m_treeWidgetShowFiles_itemClicked(QTreeWidgetItem *item, int column)
{
    QVariant itemData = item->data(0, Qt::UserRole);
    if (itemData.isValid())
    {
        emit fileItemClicked(itemData.toString());
    }
}

void RecentFilesOverview::init()
{
    updateRecentFiles();

    //禁用双击展开
    m_ui->m_treeWidgetShowFiles->setExpandsOnDoubleClick(false);
    //隐藏小三角
    m_ui->m_treeWidgetShowFiles->setRootIsDecorated(false);

    m_ui->m_lineEditSearch->setPlaceholderText(tr("Search file"));
}

void RecentFilesOverview::updateRecentFiles(const QString filter)
{
    m_ui->m_treeWidgetShowFiles->clear();

    const auto query = UsedResources | RecentlyUsedFirst | Agent::any() | Type::files() | Activity::any() | Url::contains(filter);

    for (const ResultSet::Result &result : ResultSet(query))
    {
        //        KLOG_INFO() << result.title() << result.resource();
        QString filePath = QUrl(result.resource()).path();
        if (!QFile::exists(filePath))
        {
            continue;
        }
        QIcon icon = QFileIconProvider().icon(QFileInfo(filePath));

        QTreeWidgetItem *item = new QTreeWidgetItem(m_ui->m_treeWidgetShowFiles);
        item->setIcon(0, icon);
        item->setText(0, result.title());
        item->setData(0, Qt::UserRole, filePath);
    }
}

void RecentFilesOverview::showEvent(QShowEvent *event)
{
    updateRecentFiles(m_ui->m_lineEditSearch->text());
}

void RecentFilesOverview::on_m_lineEditSearch_textChanged(const QString &arg1)
{
    updateRecentFiles(arg1);
}

}  // namespace Menu

}  // namespace Kiran
