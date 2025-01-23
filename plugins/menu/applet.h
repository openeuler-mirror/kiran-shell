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

#include <kiran-color-block.h>

#include "lib/widgets/applet-button.h"
#include "lib/widgets/styled-button.h"
#include "plugin-i.h"

namespace Kiran
{
class IAppletImport;

namespace Menu
{
class Window;

class Applet : public KiranColorBlock
{
    Q_OBJECT

public:
    Applet(IAppletImport *import);
    ~Applet();

private:
    void initializeTranslator();
    void setupWindow();
    void setupAppletButton();
    void setupLayout();

    void clickButton(bool checked);
    void hideMenu();

private:
    IAppletImport *m_import;
    Window *m_window;
    StyledButton *m_appletButton;
};

class Plugin : public QObject, public IPlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID IAPPLET_IID FILE "menu.json")
    Q_INTERFACES(Kiran::IPlugin)

public:
    virtual QWidget *createApplet(const QString &appletID, IAppletImport *import)
    {
        return new Applet(import);
    }
};

}  // namespace Menu
}  // namespace Kiran
