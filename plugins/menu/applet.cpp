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

#include <ks-i.h>
#include <qt5-log-i.h>
#include <QGridLayout>
#include <QPainter>
#include <QPixmap>
#include <QRect>
#include <QTranslator>

#include "applet.h"
#include "ks-config.h"
#include "lib/common/define.h"
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

    //    setRadius(0);

    m_window = new Window();
    connect(m_window, &Window::windowDeactivated, this, &Applet::hideMenu);

    auto size = m_import->getPanel()->getSize();
    setFixedSize(size, size);

    m_appletButton = new StyledButton(this);
    //int iconSize = size - BUTTON_BLANK_SPACE * 2;
    //    m_appletButton->setIconSize(QSize(iconSize, iconSize));
    m_appletButton->setIconSize(QSize(24, 24));

    connect(m_appletButton, &QAbstractButton::clicked, this, &Applet::clickButton);
    //    m_appletButton->setIconFromTheme(KS_ICON_MENU);
    m_appletButton->setIcon(QIcon::fromTheme(KS_ICON_MENU));
    m_appletButton->setToolTip(tr("Start Menu"));

    QGridLayout *layout = new QGridLayout(this);
    layout->setMargin(4);
    layout->setSpacing(0);
    layout->addWidget(m_appletButton);

    QObject *Object = dynamic_cast<QObject *>(m_import->getPanel());
    connect(Object, SIGNAL(panelProfileChanged()), this, SLOT(updateLayout()));
    updateLayout();
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
        m_appletButton->setEnabled(false);
    }
}

void Applet::hideMenu()
{
    m_window->hide();
    m_appletButton->setEnabled(true);
    m_appletButton->setChecked(false);
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

void Applet::updateLayout()
{
    // 清理之前设置的fixed大小
    //    setMaximumWidth(QWIDGETSIZE_MAX);

    //    auto size = m_import->getPanel()->getSize();
    //    auto *lay = layout();
    //    int orientation = m_import->getPanel()->getOrientation();
    //    if (orientation == PanelOrientation::PANEL_ORIENTATION_BOTTOM ||
    //        orientation == PanelOrientation::PANEL_ORIENTATION_TOP)
    //    {
    //        lay->setContentsMargins(LAYOUT_MARGIN, 0, LAYOUT_MARGIN, 0);

    //        setFixedSize(size + LAYOUT_MARGIN * 2, size);
    //    }
    //    else
    //    {
    //        lay->setContentsMargins(0, LAYOUT_MARGIN, 0, LAYOUT_MARGIN);

    //        setFixedSize(size, size + LAYOUT_MARGIN * 2);
    //    }
}

}  // namespace Menu

}  // namespace Kiran
