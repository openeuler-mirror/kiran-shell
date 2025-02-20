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

#include "shell.h"
#include "lib/common/window-manager.h"
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
    return m_panels.value(uid);
}

Shell::Shell() = default;

void Shell::init()
{
    // 窗口管理单例初始化,工作区 任务栏需要
    WindowManagerInit;

    initChildren();
    connect(Profile::getInstance(), &Profile::panelUIDsChanged, this, &Shell::initChildren);
}

void Shell::initChildren()
{
    QMap<QString, Panel*> panelsNew;

    auto profilePanels = Profile::getInstance()->getPanels();
    for (const auto& profilePanel : profilePanels)
    {
        auto panelUID = profilePanel->getUID();
        if (!m_panels.contains(panelUID))
        {
            auto* panel = new Panel(profilePanel);
            panelsNew.insert(panelUID, panel);
        }
        else
        {
            panelsNew.insert(panelUID, m_panels.take(panelUID));
        }
    }
    // 清理不存在的面板
    while (!m_panels.empty())
    {
        auto* panel = m_panels.take(m_panels.firstKey());
        delete panel;
    }

    m_panels = panelsNew;
}

}  // namespace Kiran
