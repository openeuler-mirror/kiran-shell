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

#include "lib/widgets/applet-button.h"
#include <plugin-i.h>

namespace Kiran
{
#define BUTTON_BLANK_SPACE 6

AppletButton::AppletButton(IAppletImport *import) : m_import(import)
{
    auto size = m_import->getPanel()->getSize();
    this->setFixedSize(size, size);

    this->setCheckable(true);
    this->setFlat(true);
}

void AppletButton::setIconByName(const QString &iconName)
{
    auto buttonSize = this->size();
    this->setIcon(QIcon(iconName));
    this->setIconSize(QSize(buttonSize.width() - BUTTON_BLANK_SPACE * 2,
                            buttonSize.height() - BUTTON_BLANK_SPACE * 2));
}

void AppletButton::setIconFromTheme(const QString &iconName)
{
    auto buttonSize = this->size();
    this->setIcon(QIcon::fromTheme(iconName));
    this->setIconSize(QSize(buttonSize.width() - BUTTON_BLANK_SPACE * 2,
                            buttonSize.height() - BUTTON_BLANK_SPACE * 2));
}

}  // namespace Kiran
