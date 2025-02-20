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

#pragma once

#include <QtPlugin>

namespace Kiran
{
#define IAPPLET_IID "com.kylinsec.Kiran.Shell.IApplet"

class IApplet
{
public:
    // virtual void setup(IAppletArgs *args) = 0;
    // // 获取Applet对应的控件对象
    // virtual QWidget *widget() = 0;
};

class IPanel
{
public:
    virtual int getSize() = 0;
    virtual int getOrientation() = 0;

    // 此接口实际作为信号用
    virtual void panelProfileChanged() = 0;
};

// 用于给插件内的applet提供插件外的设置参数。
class IAppletImport
{
public:
    virtual IPanel *getPanel() = 0;
    virtual IApplet *getApplet() = 0;
};

class IPlugin
{
public:
    virtual ~IPlugin() = default;

    virtual QWidget *createApplet(const QString &appletID, IAppletImport *import) = 0;
};

}  // namespace Kiran

Q_DECLARE_INTERFACE(Kiran::IPlugin, IAPPLET_IID)
