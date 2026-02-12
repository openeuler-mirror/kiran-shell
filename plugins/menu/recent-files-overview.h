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
class RecentFilesView;
class RecentFilesOverview : public QWidget
{
    Q_OBJECT

public:
    RecentFilesOverview(QWidget* parent = nullptr);
    ~RecentFilesOverview() override;

    class RecentFilesView* getTreeView() const;

protected:
    void showEvent(QShowEvent* event) override;

signals:
    void fileItemClicked(QString filePath);

private:
    Ui::RecentFilesOverview* m_ui;
};
}  // namespace Menu
}  // namespace Kiran
