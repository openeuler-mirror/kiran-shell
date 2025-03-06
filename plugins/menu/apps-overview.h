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

#include <QSet>
#include <QWidget>

namespace Ui
{
class AppsOverview;
};  // namespace Ui

class QGSettings;
class QTreeWidgetItem;
class KSycocaEntry;

namespace Kiran
{
namespace Menu
{
class AppsOverview : public QWidget
{
    Q_OBJECT

public:
    AppsOverview(QWidget* parent = nullptr);
    ~AppsOverview() override;

protected:
    void showEvent(QShowEvent* event) override;

private slots:
    // 应用项点击
    void on_treeWidgetApps_itemClicked(QTreeWidgetItem* item, int column);
    // 应用项右键
    void on_treeWidgetApps_itemPressed(QTreeWidgetItem* item, int column);
    // 应用搜索
    void on_lineEditSearch_textChanged(const QString& arg1);
    // 应用更新
    void updateApp();

private:
    // 应用加载和管理
    void loadApps();
    void addGroup(KSycocaEntry* entry, QString filter = "", QTreeWidgetItem* parent = nullptr);
    void addItem(KSycocaEntry* entry, QString filter = "", QTreeWidgetItem* parent = nullptr);

    // 新应用更新
    void updateNewApp(QString key = "");
    void clearNewApp();

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

    QSet<QString> m_appIds;             // 缓存所有的应用id
    bool m_isReayToloadNewApp = false;  // 已准备好载入新应用，下次载入应用列表时，先处理新应用

    QGSettings* m_gsettings;  // gsettings
};
}  // namespace Menu
}  // namespace Kiran
