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

#include "shell.h"
#include <QGlobalStatic>
#include <QScopedPointer>
#include "panel.h"
#include "profile/profile.h"

namespace Kiran
{
Shell* Shell::m_instance = nullptr;
void Shell::globalInit()
{
    m_instance = new Shell();
    m_instance->init();
}

void Shell::globalDeinit()
{
    delete m_instance;
}

Panel* Shell::getPanel(const QString& uid)
{
    return this->m_panels.value(uid);
}

Shell::Shell()
{
}

void Shell::init()
{
    auto profilePanels = Profile::getInstance()->getPanels();

    for (const auto& profilePanel : profilePanels)
    {
        auto panel = new Panel(profilePanel);
        this->m_panels.insert(panel->getUID(), panel);
    }
}

}  // namespace Kiran
