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

#include <qt5-log-i.h>
#include <QCoreApplication>
#include <QSizePolicy>
#include <QTranslator>

#include "applet.h"
#include "ks-config.h"
#include "ks-i.h"
#include "lib/common/logging-category.h"
#include "lib/common/utility.h"
#include "lib/common/window-info-helper.h"
#include "lib/common/window-manager.h"
#include "plugin-i.h"
namespace Kiran
{
namespace Taskbar
{
Applet::Applet(IAppletImport *import)
    : m_import(import)
{
    static QTranslator translator;
    if (!translator.load(QLocale(), "taskbar", ".", KS_INSTALL_TRANSLATIONDIR, ".qm"))
    {
        KLOG_WARNING(LCTaskbar) << "Load translator failed!";
    }
    else
    {
        QCoreApplication::installTranslator(&translator);
    }

    m_window = new Window(m_import, this);
    QGridLayout *gridLayout = new QGridLayout(this);
    gridLayout->setMargin(0);
    gridLayout->setSpacing(0);
    gridLayout->addWidget(m_window);
    // 最大化
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(&WindowManagerInstance, &Common::WindowManager::windowAdded, this, &Applet::windowAdded);
    connect(&WindowManagerInstance, &Common::WindowManager::windowRemoved, this, &Applet::windowRemoved);
    connect(&WindowManagerInstance, &Common::WindowManager::activeWindowChanged, this, &Applet::activeWindowChanged);
    connect(&WindowManagerInstance, &Common::WindowManager::windowChanged, m_window, &Window::windowChanged);
}

Applet::~Applet()
{
}

}  // namespace Taskbar

}  // namespace Kiran
