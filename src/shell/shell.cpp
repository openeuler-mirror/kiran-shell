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

#include "src/shell/shell.h"
#include <QGlobalStatic>
#include <QScopedPointer>
#include "src/shell/layout.h"
#include "src/shell/panel.h"

namespace Kiran
{
Q_GLOBAL_STATIC(Shell, gs_shell)
Shell* Shell::getInstance()
{
    return gs_shell;
}

Shell::Shell()
{
    this->initUI();
}

void Shell::initUI()
{
    auto panelsModel = Model::Layout::getInstance()->getPanels();

    for (const auto& panelModel : panelsModel)
    {
        auto panel = QSharedPointer<Panel>(new Panel(panelModel));
        this->m_panels.push_back(panel);
        panel->show();
    }
}

}  // namespace Kiran