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

#include <KWindowSystem/KWindowSystem>

#include "showdesktop.h"

namespace Kiran
{
Showdesktop::Showdesktop(IAppletImport *import)
    : m_import(import)
{
    connect(this, &QPushButton::clicked, this, [=]()
            { KWindowSystem::setShowingDesktop(!KWindowSystem::showingDesktop()); });

    auto size = m_import->getPanel()->getSize();
    //    setFixedSize(size, size);
    setMinimumSize(size, size);
    setMaximumSize(size, size);

    setToolTip(tr("Show desktop"));
}

}  // namespace Kiran
