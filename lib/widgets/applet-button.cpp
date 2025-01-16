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

#include <plugin-i.h>

#include "applet-button.h"
#include "ks-i.h"

namespace Kiran
{
AppletButton::AppletButton(IAppletImport *import)
    : m_import(import)
{
    auto size = m_import->getPanel()->getSize();
    setFixedSize(size, size);

    setCheckable(true);
    setFlat(true);
}

AppletButton::~AppletButton()
{
}

void AppletButton::setIconByName(const QString &iconName)
{
    auto buttonSize = size();
    setIconSize(QSize(buttonSize.width() - BUTTON_BLANK_SPACE * 2,
                      buttonSize.height() - BUTTON_BLANK_SPACE * 2));

    setIcon(QIcon(iconName));
}

void AppletButton::setIconFromTheme(const QString &iconName)
{
    auto buttonSize = size();
    setIconSize(QSize(buttonSize.width() - BUTTON_BLANK_SPACE * 2,
                      buttonSize.height() - BUTTON_BLANK_SPACE * 2));

    setIcon(QIcon::fromTheme(iconName));
}

}  // namespace Kiran
