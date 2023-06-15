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

#include "src/shell/applet.h"
#include <qt5-log-i.h>
#include "src/shell/layout.h"
#include "src/shell/plugin.h"

namespace Kiran
{
// Applet::Applet(const QString &uid, QObject *parent) : QObject(parent),
//                                                       m_uid(uid)
// {
// }

// QWidget *Applet::build()
// {
//     auto plugin = PluginPool::getInstance()->findPluginForApplet(this->m_id);
//     if (!plugin)
//     {
//         KLOG_WARNING() << "Not found plugin for applet id: " << this->m_id;
//         return nullptr;
//     }

//     return plugin->createApplet(this->m_id);
// }

}  // namespace Kiran
