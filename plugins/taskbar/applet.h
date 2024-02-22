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

#include <kiran-color-block.h>
#include <plugin-i.h>
#include <QBoxLayout>
#include <QWidget>

#include "app-button-container.h"

namespace Kiran
{
namespace Taskbar
{
class Applet : public /*QWidget*/ KiranColorBlock
{
    Q_OBJECT

public:
    Applet(IAppletImport *import);
    ~Applet();

private slots:
    //  打开或关闭窗口软件
    void addWindow(WId id);
    void removeWindow(WId id);
    //激活窗口
    void changedActiveWindow(WId id);
    // 重新获取app，并显示
    void updateAppShow();
    // panel布局信息发生变化
    void updateLayout();
    // 获取panel方向信息
    QBoxLayout::Direction getLayoutDirection();
    Qt::AlignmentFlag getLayoutAlignment();

signals:
    //  打开或关闭窗口软件
    void windowAdded(WId id);
    void windowRemoved(WId id);
    void activeWindowChanged(WId id);

private:
    IAppletImport *m_import;

    // app容器
    AppButtonContainer *m_appButtonContainer;

    QBoxLayout *m_layout;
};

class Plugin : public QObject, public IPlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID IAPPLET_IID FILE "taskbar.json")
    Q_INTERFACES(Kiran::IPlugin)

public:
    virtual QWidget *createApplet(const QString &appletID, IAppletImport *import)
    {
        return new Applet(import);
    }
};

}  // namespace Taskbar

}  // namespace Kiran
