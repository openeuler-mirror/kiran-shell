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
 * Author:     yangfeng <yangfeng@kylinsec.com.cn>
 */
#pragma once
#include <QTreeView>

namespace Kiran
{
namespace Menu
{
class TreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit TreeView(QWidget* parent = nullptr);
    virtual ~TreeView();

protected:
    using QTreeView::setModel;
};

class AppsModel;
class AppsView : public TreeView
{
    Q_OBJECT
public:
    AppsView(QWidget* parent = nullptr);
    ~AppsView() override;

signals:
    // 查询是否在收藏夹中
    void isInFavorite(const QString& appId, bool& checkResult);
    // 查询是否已固定到任务栏
    void isInFixedApps(const QUrl& url, bool& checkResult);
    void runApp(const QString& appID);
    // 添加到×/从×移除 桌面、收藏夹、任务栏
    void addToDesktop(const QString& appId);
    void addToFavorite(const QString& appId);
    void removeFromFavorite(const QString& appId);
    void addToFixedApps(const QUrl& url);
    void removeFromFixedApps(const QUrl& url);

public slots:
    void setFilterText(const QString& text);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    // 右键拖动起始位置，用于防止误触，当移动坐标达到阈值之后才判定为拖拽
    QPoint m_pressPoint;
    AppsModel* m_model = nullptr;
};

class RecentFilesModel;
class RecentFilesView : public TreeView
{
    Q_OBJECT
public:
    RecentFilesView(QWidget* parent = nullptr);
    ~RecentFilesView() override;

public slots:
    void setFilterText(const QString& text);

signals:
    void fileItemClicked(QString filePath);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    RecentFilesModel* m_model = nullptr;
};
}  // namespace Menu
}  // namespace Kiran
