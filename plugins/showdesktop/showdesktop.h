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
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#include <plugin-i.h>
#include <QPushButton>

class QMouseEvent;

namespace Kiran
{
class Showdesktop : public QPushButton
{
    Q_OBJECT

public:
    Showdesktop();

    // virtual void setup(IAppletArgs *args){};
    // // 获取Applet对应的控件对象
    // virtual QWidget *widget() { return this; };

protected:
    virtual void mousePressEvent(QMouseEvent *event);

    // virtual void resizeEvent(QResizeEvent *event);
};

class ShowDesktopPlugin : public QObject, public IPlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID IAPPLET_IID FILE "showdesktop.json")
    Q_INTERFACES(Kiran::IPlugin)

public:
    virtual QWidget *createApplet(const QString &appletID)
    {
        return new Showdesktop();
    }
};
}  // namespace Kiran