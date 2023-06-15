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
#define IAPPLET_IID "com.kylinsec.Kiran.Shell.IApplet/2.5"

class IAppletArgs
{
public:
};

// class IApplet
// {
// public:
//     virtual void setup(IAppletArgs *args) = 0;
//     // 获取Applet对应的控件对象
//     virtual QWidget *widget() = 0;
// };

class IPlugin
{
public:
    virtual ~IPlugin(){};

    virtual QWidget *createApplet(const QString &appletID) = 0;
};

}  // namespace Kiran

Q_DECLARE_INTERFACE(Kiran::IPlugin, IAPPLET_IID)