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

#include <ks-i.h>
#include <qt5-log-i.h>
#include <QPainter>
#include <QPixmap>
#include <QRect>
#include <QTranslator>

#include "applet.h"
#include "ks-config.h"
#include "window.h"

namespace Kiran
{
namespace Menu
{
Applet::Applet(IAppletImport *import)
    : AppletButton(import),
      m_import(import)
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

    m_window = new Window();
    connect(m_window, &Window::windowDeactivated, this, &Applet::hideMenu);

    connect(this, &QAbstractButton::clicked, this, &Applet::clickButton);

    setIconFromTheme(KS_ICON_MENU);
    setToolTip(tr("Start Menu"));
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
    // KLOG_INFO() << "Applet::clickButton" << checked;
    if (checked)
    {
        updateWindowPosition();
        m_window->show();
    }
}

void Applet::hideMenu()
{
    m_window->hide();
    setChecked(false);
}

void Applet::updateWindowPosition()
{
    auto oriention = m_import->getPanel()->getOrientation();
    auto appletGeometry = geometry();
    auto windowSize = m_window->frameSize();
    QPoint windowPosition(0, 0);

    switch (oriention)
    {
    case PanelOrientation::PANEL_ORIENTATION_TOP:
        windowPosition = appletGeometry.bottomLeft();
        break;
    case PanelOrientation::PANEL_ORIENTATION_RIGHT:
        windowPosition = appletGeometry.topLeft() -= QPoint(windowSize.width(), 0);
        break;
    case PanelOrientation::PANEL_ORIENTATION_BOTTOM:
        windowPosition = appletGeometry.topLeft() -= QPoint(0, windowSize.height());
        break;
    case PanelOrientation::PANEL_ORIENTATION_LEFT:
        windowPosition = appletGeometry.topRight();
        break;
    default:
        KLOG_WARNING() << "Unknown oriention " << oriention;
        break;
    }

    m_window->move(mapToGlobal(windowPosition));
}

}  // namespace Menu

}  // namespace Kiran
