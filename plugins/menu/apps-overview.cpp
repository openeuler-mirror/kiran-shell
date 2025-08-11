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
#include <KActivities/ResourceInstance>
#include <KConfigCore/KConfigGroup>
#include <KConfigCore/KDesktopFile>
#include <KService/KServiceGroup>
#include <KSycoca>
#include <QAction>
#include <QCursor>
#include <QFileInfo>
#include <QGSettings>
#include <QMap>
#include <QMenu>
#include <QTreeWidgetItem>

#include "apps-overview.h"
#include "ks-i.h"
#include "lib/common/app-launcher.h"
#include "lib/common/logging-category.h"
#include "lib/common/utility.h"
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
    connect(m_ui->edit_search, &QLineEdit::textChanged, m_ui->treeview_apps, &AppsView::setFilterText);

    connect(m_ui->treeview_apps, &AppsView::isInFavorite, this, &AppsOverview::isInFavorite);
    connect(m_ui->treeview_apps, &AppsView::isInFixedApps, this, &AppsOverview::isInFixedApps);
    connect(m_ui->treeview_apps, &AppsView::runApp, this, &AppsOverview::runApp);
    connect(m_ui->treeview_apps, &AppsView::addToDesktop, this, &AppsOverview::addToDesktop);
    connect(m_ui->treeview_apps, &AppsView::addToFavorite, this, &AppsOverview::addToFavorite);
    connect(m_ui->treeview_apps, &AppsView::removeFromFavorite, this, &AppsOverview::removeFromFavorite);
    connect(m_ui->treeview_apps, &AppsView::addToFixedApps, this,&AppsOverview::addToFixedApps);
    connect(m_ui->treeview_apps, &AppsView::removeFromFixedApps, this, &AppsOverview::removeFromFixedApps);
}

AppsOverview::~AppsOverview()
{
    delete m_ui;
}

void AppsOverview::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    m_ui->edit_search->setFocus();
    m_ui->edit_search->clear();
}

}  // namespace Menu

}  // namespace Kiran
