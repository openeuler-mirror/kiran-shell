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

#include <plugin-i.h>

#include "applet.h"
#include "window.h"

namespace Kiran
{
namespace Systemtray
{
Applet::Applet(IAppletImport *import)
    : m_import(import)
{
    QGridLayout *layout = new QGridLayout(this);
    setLayout(layout);
    layout->setMargin(0);
    layout->setSpacing(0);

    m_window = new Window(import, this);
    layout->addWidget(m_window);
}

Applet::~Applet()
{
}

}  // namespace Systemtray

}  // namespace Kiran
