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
#include <QGridLayout>
#include <QPainter>
#include <QPixmap>
#include <QRect>
#include <QTranslator>

#include "applet.h"
#include "ks-config.h"
#include "ks-i.h"
#include "plugin-i.h"
#include "window.h"

namespace Kiran
{
namespace Workspace
{
Applet::Applet(IAppletImport *import)
    : m_import(import)
{
    static QTranslator translator;
    if (!translator.load(QLocale(), "workspace", ".", KS_INSTALL_TRANSLATIONDIR, ".qm"))
    {
        KLOG_WARNING() << "Load translator failed!";
    }
    else
    {
        QCoreApplication::installTranslator(&translator);
    }

    setRadius(0);
    m_window = new Window();
    connect(m_window, &Window::windowDeactivated, this, &Applet::hideWindow);

    auto size = m_import->getPanel()->getSize();
    setFixedSize(size, size);

    m_appletButton = new StyledButton(this);
    int iconSize = size - BUTTON_BLANK_SPACE * 2;
    m_appletButton->setIconSize(QSize(24, 24));

    connect(m_appletButton, &QAbstractButton::clicked, this, &Applet::clickButton);
    m_appletButton->setIcon(QIcon::fromTheme(KS_ICON_WORKSPACE_SWITCHER));
    setToolTip(tr("Workspace switcher"));

    QGridLayout *layout = new QGridLayout(this);
    layout->setMargin(4);
    layout->setSpacing(0);
    layout->addWidget(m_appletButton);
}

Applet::~Applet()
{
    if (m_window)
    {
        delete m_window;
        m_window = nullptr;
    }
}

void Applet::clickButton(bool checked)
{
    if (checked)
    {
        m_window->show();
        // 防止再次点击,当窗口隐藏时,解除禁用
        m_appletButton->setEnabled(false);
    }
}

void Applet::hideWindow()
{
    m_window->hide();
    m_appletButton->setEnabled(true);
    m_appletButton->setChecked(false);
}

}  // namespace Workspace

}  // namespace Kiran
