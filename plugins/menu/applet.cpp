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
#include <QCoreApplication>
#include <QGridLayout>
#include <QTranslator>

#include "applet.h"
#include "ks-config.h"
#include "ks-i.h"
#include "lib/common/logging-category.h"
#include "lib/common/utility.h"
#include "window.h"
#include "shell_menu_adaptor.h"

#define LAYOUT_MARGIN 4
#define SHELL_BUS "com.kylinsec.Kiran.Shell"
#define SHELL_BUS_MENU_PATH "/com/kylinsec/Kiran/Shell/Menu"

namespace Kiran
{
namespace Menu
{
Applet::Applet(IAppletImport *import)
    : m_import(import),
      m_window(nullptr),
      m_appletButton(nullptr)
{
    initializeTranslator();
    setupWindow();
    setupAppletButton();
    setupLayout();
    setupDbus();
}

Applet::~Applet()
{
}

void Applet::activateStartMenu()
{
    if (!m_window->isVisible())
    {
        KLOG_INFO(LCMenu) << "show menu by dbus";
        showMenu();
    }
    else
    {
        KLOG_INFO(LCMenu) << "hide menu by dbus";
        hideMenu();
    }
}

void Applet::initializeTranslator()
{
    static QTranslator translator;
    if (!translator.load(QLocale(), "menu", ".", KS_INSTALL_TRANSLATIONDIR, ".qm"))
    {
        KLOG_WARNING(LCMenu) << "Load translator failed!";
    }
    else
    {
        QCoreApplication::installTranslator(&translator);
    }
}

void Applet::setupWindow()
{
    m_window = new Window(this);
    connect(m_window, &Window::windowDeactivated, this, &Applet::hideMenu);
}

void Applet::setupAppletButton()
{
    m_appletButton = new StyledButton(this);
    m_appletButton->setIconSize(QSize(PANEL_APP_ICON_SIZE, PANEL_APP_ICON_SIZE));
    m_appletButton->setIcon(QIcon::fromTheme(KS_ICON_MENU));
    m_appletButton->setToolTip(tr("Start Menu"));

    connect(m_appletButton, &QAbstractButton::clicked, this, &Applet::clickButton);
}

void Applet::setupLayout()
{
    QGridLayout *layout = new QGridLayout(this);
    layout->setMargin(4);
    layout->setSpacing(0);
    layout->addWidget(m_appletButton);

    setRadius(0);
    auto size = m_import->getPanel()->getSize();
    setFixedSize(size, size);
}

void Applet::setupDbus()
{
    new ShellMenuAdaptor(this);

    auto sessionConnection = QDBusConnection::sessionBus();
    if (!sessionConnection.registerService(SHELL_BUS))
    {
        KLOG_WARNING() << "Failed to register dbus name: " << SHELL_BUS << sessionConnection.lastError();
    }

    if (!sessionConnection.registerObject(SHELL_BUS_MENU_PATH, this))
    {
        KLOG_WARNING() << "Can't register object:" << SHELL_BUS_MENU_PATH << sessionConnection.lastError();
    }
}

void Applet::clickButton(bool checked)
{
    if (checked)
    {
        showMenu();
    }
}

void Applet::showMenu()
{
    m_window->show();
    m_appletButton->setEnabled(false);
    m_appletButton->setChecked(true);

    auto oriention = m_import->getPanel()->getOrientation();
    Utility::updatePopWidgetPos(oriention, this, m_window);

    // 激活开始菜单
    m_window->raise();
    m_window->activateWindow();
}

void Applet::hideMenu()
{
    m_window->hide();
    m_appletButton->setEnabled(true);
    m_appletButton->setChecked(false);
}

}  // namespace Menu

}  // namespace Kiran
