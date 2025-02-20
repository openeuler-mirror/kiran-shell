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

#include <QWidget>

#include "plugin-i.h"
namespace Kiran
{
class ProfileApplet;
class Applet;
class Panel;

class AppletImport : public QObject, public IAppletImport
{
    Q_OBJECT

public:
    AppletImport(Applet *applet, QObject *parent = nullptr);

public:
    IPanel *getPanel() override;
    IApplet *getApplet() override;

private:
    Applet *m_applet;
};

class Applet : public QWidget, public IApplet
{
    Q_OBJECT

public:
    Applet(ProfileApplet *profileApplet, Panel *panel);

public:
    Panel *getPanel();
    QString getID();

private:
    void init();

private:
    ProfileApplet *m_profileApplet = nullptr;
    Panel *m_panel = nullptr;
    // 提供给插件的输入对象
    AppletImport *m_appletImport = nullptr;
    QWidget *m_pluginApplet = nullptr;
};
}  // namespace Kiran
