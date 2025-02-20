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

#include <KWindowInfo>
#include <QWidget>

#include "lib/widgets/window-thumbnail.h"

class QMenu;

namespace Kiran
{
class IAppletImport;

namespace Taskbar
{
class AppPreviewer;

class WindowPreviewer : public WindowThumbnail
{
    Q_OBJECT

public:
    explicit WindowPreviewer(WId wid, IAppletImport *import, AppPreviewer *parent = nullptr);
    ~WindowPreviewer() override;

    // 当弹出菜单时，父窗口会检测到leaveEvent，
    // 此处判断是否可以隐藏，如果菜单已弹出，则在菜单执行结束时隐藏窗口
    bool checkCanHide();

private:
    // 监测窗口变化
    void changedActiveWindow(WId wid);

private slots:
    void on_btnClose_clicked() override;

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

signals:
    void closeWindow(WId wid);
    void hideWindow();

private:
    IAppletImport *m_import;

    // 记录上次激活的窗口，预览窗口点击时，会将预览窗口激活，导致无法隐藏对应的应用窗口
    WId m_widLastActive;

    QMenu *m_menu;
};

}  // namespace Taskbar

}  // namespace Kiran
