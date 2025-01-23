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

#pragma once

#include <QTreeWidgetItem>
#include <QWidget>

#include <KActivities/Stats/ResultModel>
#include <KActivities/Stats/ResultSet>
#include <KActivities/Stats/ResultWatcher>
#include <KService/KService>

namespace Ui
{
class RecentFilesOverview;
};  // namespace Ui

namespace Kiran
{
namespace Menu
{
class RecentFilesOverview : public QWidget
{
    Q_OBJECT

public:
    RecentFilesOverview(QWidget* parent = nullptr);
    virtual ~RecentFilesOverview();

private slots:
    void on_treeWidgetShowFiles_itemClicked(QTreeWidgetItem* item, int column);
    void on_lineEditSearch_textChanged(const QString& arg1);

private:
    void updateRecentFiles(const QString filter = "");

protected:
    void showEvent(QShowEvent* event);
signals:
    void fileItemClicked(QString filePath);

private:
    Ui::RecentFilesOverview* m_ui;

    // 监控常用文档变化
    KActivities::Stats::ResultWatcher* m_actStatsWatcher;
};
}  // namespace Menu
}  // namespace Kiran
