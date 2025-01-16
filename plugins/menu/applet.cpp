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

#include <qt5-log-i.h>
#include <QGridLayout>
#include <QPainter>
#include <QPixmap>
#include <QRect>
#include <QTranslator>

#include "applet.h"
#include "ks-config.h"
#include "ks-i.h"
#include "lib/common/utility.h"
#include "window.h"

#define LAYOUT_MARGIN 4

namespace Kiran
{
namespace Menu
{
Applet::Applet(IAppletImport *import)
    : m_import(import)
{
    static QTranslator translator;
    if (!translator.load(QLocale(), "menu", ".", KS_INSTALL_TRANSLATIONDIR, ".qm"))
    {
        KLOG_WARNING() << "Load translator failed!";
    }
    else
    {
        QCoreApplication::installTranslator(&translator);
    }

    m_window = new Window(this);
    connect(m_window, &Window::windowDeactivated, this, &Applet::hideMenu);

    setRadius(0);
    auto size = m_import->getPanel()->getSize();
    setFixedSize(size, size);

    m_appletButton = new StyledButton(this);
    m_appletButton->setIconSize(QSize(24, 24));

    connect(m_appletButton, &QAbstractButton::clicked, this, &Applet::clickButton);
    m_appletButton->setIcon(QIcon::fromTheme(KS_ICON_MENU));
    m_appletButton->setToolTip(tr("Start Menu"));

    QGridLayout *layout = new QGridLayout(this);
    layout->setMargin(4);
    layout->setSpacing(0);
    layout->addWidget(m_appletButton);
}

Applet::~Applet()
{
}

void Applet::clickButton(bool checked)
{
    // KLOG_INFO() << "Applet::clickButton" << checked;
    if (checked)
    {
        m_window->show();
        m_appletButton->setEnabled(false);

        auto oriention = m_import->getPanel()->getOrientation();
        Utility::updatePopWidgetPos(oriention, this, m_window);
    }
}

void Applet::hideMenu()
{
    m_window->hide();
    m_appletButton->setEnabled(true);
    m_appletButton->setChecked(false);
}

}  // namespace Menu

}  // namespace Kiran
