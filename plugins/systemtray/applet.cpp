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
 * Author:     yangfeng <yangfeng@kylinsec.com.cn>
 */

#include "applet.h"
#include "plugin-i.h"
#include "tray.h"

namespace Kiran
{
namespace Systemtray
{
Applet::Applet(IAppletImport *import)
    : m_import(import)
{
    m_tray = new Tray(import, this);

    auto *layout = new QGridLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(m_tray);
}

Applet::~Applet()
{
}

}  // namespace Systemtray

}  // namespace Kiran
