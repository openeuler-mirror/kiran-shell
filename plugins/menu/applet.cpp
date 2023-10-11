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

#include "plugins/menu/applet.h"
#include <ks-i.h>
#include <plugin-i.h>
#include <qt5-log-i.h>
#include <QPainter>
#include <QPixmap>
#include <QRect>
#include "plugins/menu/window.h"

namespace Kiran
{
namespace Menu
{
Applet::Applet(IAppletImport *import) : AppletButton(import),
                                        m_import(import)
{
    this->m_window = new Window();

    auto size = m_import->getPanel()->getSize();
    this->setFixedSize(size, size);
    this->setCheckable(true);
    this->setFlat(true);
    this->setIconFromTheme(KS_ICON_THEME_MENU);

    connect(this, &QAbstractButton::toggled, this, &Applet::setChecked);
}

// void Applet::paintEvent(QPaintEvent *event)
// {
//     QPainter painter(this);
//     QPixmap icon(":/images/logo");
//     auto scaledIcon = icon.scaled(30, 30);

//     painter.drawPixmap(0, 0, scaledIcon);
//     QWidget::paintEvent(event);
// }

void Applet::setChecked(bool checked)
{
    if (checked)
    {
        this->updateWindowPosition();
        this->m_window->show();
    }
    else
    {
        this->m_window->hide();
    }
}

void Applet::updateWindowPosition()
{
    auto oriention = this->m_import->getPanel()->getOrientation();
    auto appletGeometry = this->geometry();
    auto windowSize = this->m_window->frameSize();
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

    this->m_window->move(this->mapToGlobal(windowPosition));
}

}  // namespace Menu

}  // namespace Kiran
