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

#include <QWidget>

#include "plugin-i.h"
#include "window.h"

namespace Kiran
{
namespace Taskbar
{
class Applet : public QWidget
{
    Q_OBJECT

public:
    Applet(IAppletImport *import);
    ~Applet();

signals:
    //  打开或关闭窗口软件
    void windowAdded(WId wid);
    void windowRemoved(WId wid);
    void activeWindowChanged(WId wid);

private:
    IAppletImport *m_import;

    // app容器
    Window *m_window;
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
