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
 * Author:     yangfeng <yangfeng@kylinsec.com.cn>
 */

#pragma once

#include <KWindowInfo>
#include <QWidget>

class QMenu;

namespace Ui
{
class WindowPreviewer;
}

namespace Kiran
{
class IAppletImport;

namespace Taskbar
{
class AppPreviewer;

class WindowPreviewer : public QWidget
{
    Q_OBJECT

public:
    explicit WindowPreviewer(WId wid, IAppletImport *import, AppPreviewer *parent = nullptr);
    ~WindowPreviewer();

    // 当弹出菜单时，父窗口会检测到leaveEvent，
    // 此处判断是否可以隐藏，如果菜单已弹出，则在菜单执行结束时隐藏窗口
    bool checkCanHide();

    // 获取截图，更新显示
    void updatePreviewer();

private:
    // 监测窗口变化
    void changedWindow(WId wid, NET::Properties properties, NET::Properties2 properties2);
    void changedActiveWindow(WId wid);

    // 启动截图
    void startUpdatePreviewer();

private slots:
    void on_m_btnClose_clicked();

protected:
    void mousePressEvent(QMouseEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    void showEvent(QShowEvent *event);

signals:
    void closeWindow(WId wid);
    void hideWindow();

private:
    Ui::WindowPreviewer *m_ui;
    IAppletImport *m_import;

    WId m_wid;
    // 记录上次激活的窗口，预览窗口点击时，会将预览窗口激活，导致无法隐藏对应的应用窗口
    WId m_widLastActive;

    QMenu *m_menu;

    bool m_updateInProgress;

    QPixmap m_pixPreviewer;
};

}  // namespace Taskbar

}  // namespace Kiran
