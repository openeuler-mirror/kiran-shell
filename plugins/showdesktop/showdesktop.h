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

#include <plugin-i.h>
#include <QPushButton>

class QMouseEvent;

namespace Kiran
{
class IAppletImport;

class Showdesktop : public QPushButton
{
    Q_OBJECT

public:
    Showdesktop(IAppletImport *import);

    // virtual void setup(IAppletArgs *args){};
    // // 获取Applet对应的控件对象
    // virtual QWidget *widget() { return this; };
private slots:
    // panel布局信息发生变化
    void updateLayout();

private:
    IAppletImport *m_import;
};

class ShowDesktopPlugin : public QObject, public IPlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID IAPPLET_IID FILE "showdesktop.json")
    Q_INTERFACES(Kiran::IPlugin)

public:
    virtual QWidget *createApplet(const QString &appletID, IAppletImport *import)
    {
        return new Showdesktop(import);
    }
};
}  // namespace Kiran
