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

qdbuscpp2xml会解析QObject派生类的C++头文件或是源文件，生成D-Bus的内省xml文件  
qdbusxml2cpp可以辅助自动生成继承于QDBusAbstractAdaptor和QDBusAbstractInterface两个类的实现代码，用于进程通信服务端和客户端，简化了开发者的代码设计  

dbus xml文件生成  
qdbuscpp2xml status-notifier-watcher.h -A -o org.kde.StatusNotifierWatcher.xml  

dbus adaptor类生成，用于服务端，-a，方便注册object，用于发布服务  
qdbusxml2cpp org.kde.StatusNotifierWatcher.xml -a dbus-adaptor  

dbus proxy类(interface)生成，用于服务端，-p，用于访问Service，如同调用本地对象的方法一样  
qdbusxml2cpp org.kde.StatusNotifierWatcher.xml -p StatusNotifierWatcherInterface  
