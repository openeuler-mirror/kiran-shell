/**
 * Copyright (c) 2024 ~ 2025 KylinSec Co., Ltd.
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

namespace Kiran
{
namespace SettingBar
{
class Window;
class Applet : public QWidget
{
    Q_OBJECT

public:
    Applet(IAppletImport *import);
    ~Applet() override;

private:
    IAppletImport *m_import;

    Window *m_window;
};

class Plugin : public QObject, public IPlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID IAPPLET_IID FILE "settingbar.json")
    Q_INTERFACES(Kiran::IPlugin)

public:
    QWidget *createApplet(const QString &appletID, IAppletImport *import) override
    {
        return new Applet(import);
    }
};
}  // namespace SettingBar
}  // namespace Kiran
