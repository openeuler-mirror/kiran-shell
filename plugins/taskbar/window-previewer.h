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

#include <QMenu>
#include <QWidget>

namespace Ui
{
class WindowPreviewer;
}

namespace Kiran
{
namespace Taskbar
{
class WindowPreviewer : public QWidget
{
    Q_OBJECT

public:
    explicit WindowPreviewer(WId wid, QWidget *parent = nullptr);
    ~WindowPreviewer();

    // 获取截图，更新显示
    void updatePreviewer();
    // 当弹出菜单时，父窗口会检测到leaveEvent，
    // 此处判断是否可以隐藏，如果菜单已弹出，则在菜单执行结束时隐藏窗口
    bool checkCanHide();

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

    WId m_wid;
    // 记录上次激活的窗口，预览窗口点击时，会将预览窗口激活，导致无法隐藏对应的应用窗口
    WId m_widLastActive;

    QMenu *m_menu;
};

}  // namespace Taskbar

}  // namespace Kiran
