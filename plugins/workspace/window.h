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

#include <QMap>
#include <QWidget>

namespace Ui
{
class Window;
}

class QListWidgetItem;
namespace Kiran
{
namespace Workspace
{
class DesktopHelper;
class WorkspaceThumbnail;
class WorkspaceOverview;

// 工作区主窗口
class Window : public QWidget
{
    Q_OBJECT

public:
    explicit Window(QWidget *parent = nullptr);
    ~Window();

protected:
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

    // 事件过滤器
    bool eventFilter(QObject *object, QEvent *event) override;

private slots:
    void on_btnAddWorkspace_clicked();

    void on_listWidgetThumbnail_itemDoubleClicked(QListWidgetItem *item);

    void on_listWidgetThumbnail_currentRowChanged(int currentRow);

private:
    void init();

    // 关于工作区和桌面的叫法问题
    // kf5提供的接口是desktop
    // kiran-shell插件是workspace
    // 在编码中混用两种,都是一个意思

    // 创建一个工作区
    void createDesktop();
    // 移除一个工作区
    void removeDesktop(int desktop);

    // 已经存在的工作区,添加到界面
    void addWorkspace(int desktop);
    void clearWorkspace();

    // 界面呈现的桌面变化(即显示某个桌面所包含的应用)
    void changeCurrentDesktop(int desktop);
    // 界面呈现的桌面数变化
    void changeNumberOfDesktops(int numOfDesk);

signals:
    void windowDeactivated();

private:
    Ui::Window *m_ui;

    DesktopHelper *m_desktopHelper;
    //    QList<WorkspaceThumbnail *> m_workspaces;
    QMap<int, QPair<WorkspaceThumbnail *, WorkspaceOverview *>> m_workspaces;  // 桌面编号, <桌面缩略图, 桌面预览图>
};
}  // namespace Workspace
}  // namespace Kiran
