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
      m_ui(new Ui::RecentFilesOverview)
{
    m_ui->setupUi(this);
    connect(m_ui->edit_search, &QLineEdit::textChanged,m_ui->tree_recentFiles, &RecentFilesView::setFilterText);
    connect(m_ui->tree_recentFiles,&RecentFilesView::fileItemClicked,this, &RecentFilesOverview::fileItemClicked);
}

RecentFilesOverview::~RecentFilesOverview()
{
    delete m_ui;
}

}  // namespace Menu

}  // namespace Kiran
