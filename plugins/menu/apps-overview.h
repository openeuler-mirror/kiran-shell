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

#pragma once

#include <KService/KServiceGroup>
#include <QTreeWidgetItem>
#include <QWidget>

namespace Ui
{
class AppsOverview;
};  // namespace Ui

namespace Kiran
{
namespace Menu
{
class AppsOverview : public QWidget
{
    Q_OBJECT

public:
    AppsOverview(QWidget* parent = nullptr);
    virtual ~AppsOverview();

private slots:
    // 应用项点击
    void on_m_treeWidgetApps_itemClicked(QTreeWidgetItem* item, int column);
    // 应用项右键
    void on_m_treeWidgetApps_itemPressed(QTreeWidgetItem* item, int column);

    void on_m_lineEditSearch_textChanged(const QString& arg1);

protected:
    void showEvent(QShowEvent* event) override;

private:
    void init();

    // 加载应用
    void loadApps();
    //遍历应用列表
    void recursiveService(KServiceGroup* serviceGroup, const QString& filter = "", QTreeWidgetItem* parent = nullptr);
    //增加应用
    void addItem(KSycocaEntry* entry, const QString filter = "", QTreeWidgetItem* parent = nullptr);
signals:
    // 查询是否在收藏夹中
    void isInFavorite(const QString& appId, bool& checkResult);
    // 查询是否已固定到任务栏
    void isInTasklist(const QUrl& url, bool& checkResult);

    // 添加到×/从×移除 桌面、收藏夹、任务栏
    void addToDesktop(const QString& appId);
    void addToFavorite(const QString& appId);
    void removeFromFavorite(const QString& appId);
    void addToTasklist(const QUrl& url);
    void removeFromTasklist(const QUrl& url);

    // 运行应用
    void runApp(const QString& appId);

private:
    Ui::AppsOverview* m_ui;
};
}  // namespace Menu
}  // namespace Kiran
