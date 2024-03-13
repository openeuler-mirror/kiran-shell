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

#include <plugin-i.h>
#include <QMap>
#include <QSharedPointer>

class QPluginLoader;

namespace Kiran
{
class PluginInfo;

// 插件池管理所有与当前布局相关的插件，应该只有在需要时才进行加载
class PluginPool
{
public:
    static PluginPool *getInstance();

    PluginPool();

    // 如果已经存在则直接返回，否则搜索所有插件的meta信息来找到对应的插件，查找meta信息的过程是不需要加载插件的
    IPlugin *findPluginForApplet(const QString &appletID);

private:
    void loadPluginsMeta();

private:
    // <插件路径，插件对象>
    QMap<QString, QSharedPointer<PluginInfo>> m_plugins;
};
}  // namespace Kiran
