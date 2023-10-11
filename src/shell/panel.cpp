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

#include "src/shell/panel.h"
#include <ks-i.h>
#include <qt5-log-i.h>
#include <KWindowSystem/KWindowSystem>
#include <QApplication>
#include <QBoxLayout>
#include <QScopedPointer>
#include <QScreen>
#include "src/shell/applet.h"
#include "src/shell/profile/profile.h"
#include "src/shell/utils.h"

namespace Kiran
{
Panel::Panel(ProfilePanel* profilePanel) : QWidget(nullptr, Qt::FramelessWindowHint),
                                           m_profilePanel(profilePanel)
{
    this->setAttribute(Qt::WA_X11NetWmWindowTypeDock);
    this->m_orientation = this->orientationStr2Enum(this->m_profilePanel->getOrientation());

    this->init();
}

QString Panel::getUID()
{
    return this->m_profilePanel->getUID();
}

int Panel::getSize()
{
    return this->m_profilePanel->getSize();
}

int Panel::getOrientation()
{
    return int(this->orientationStr2Enum(this->m_profilePanel->getOrientation()));
}

void Panel::init()
{
    this->initSelf();
    this->initChildren();
    this->show();
}

void Panel::initSelf()
{
    // 获取所在显示器
    auto monitorIndex = this->m_profilePanel->getMonitor();

    auto primaryScreen = QApplication::primaryScreen();
    auto screens = QGuiApplication::screens();
    QScreen* showingScreen = primaryScreen;

    if (monitorIndex >= screens.size())
    {
        KLOG_WARNING("The monitor index exceeds the maximum number of screens, so it will use primary screen.");
    }

    if (monitorIndex > 0 && monitorIndex < screens.size())
    {
        showingScreen = screens.at(monitorIndex);
    }

    KLOG_DEBUG() << "orientation: " << (int)this->m_orientation
                 << "x: " << showingScreen->geometry().x()
                 << "y: " << showingScreen->geometry().y()
                 << "width: " << showingScreen->geometry().width()
                 << "height: " << this->m_profilePanel->getSize();

    // 计算放置的位置，并且确保该区域不被其他窗口覆盖
    switch (this->m_orientation)
    {
    case PanelOrientation::PANEL_ORIENTATION_TOP:
        this->setGeometry(QRect(showingScreen->geometry().x(),
                                showingScreen->geometry().y(),
                                showingScreen->geometry().width(),
                                this->m_profilePanel->getSize()));

        KWindowSystem::setExtendedStrut(this->winId(),
                                        0, 0, 0,
                                        0, 0, 0,
                                        this->m_profilePanel->getSize(),
                                        showingScreen->geometry().x(),
                                        showingScreen->geometry().x() + showingScreen->geometry().width(),
                                        0, 0, 0);

        break;
    case PanelOrientation::PANEL_ORIENTATION_RIGHT:
        this->setGeometry(QRect(showingScreen->geometry().right() - this->m_profilePanel->getSize(),
                                showingScreen->geometry().y(),
                                this->m_profilePanel->getSize(),
                                showingScreen->geometry().height()));

        KWindowSystem::setExtendedStrut(this->winId(),
                                        0, 0, 0,
                                        this->m_profilePanel->getSize(),
                                        showingScreen->geometry().y(),
                                        showingScreen->geometry().y() + showingScreen->geometry().height(),
                                        0, 0, 0,
                                        0, 0, 0);
        break;
    case PanelOrientation::PANEL_ORIENTATION_LEFT:
        this->setGeometry(QRect(showingScreen->geometry().x(),
                                showingScreen->geometry().y(),
                                this->m_profilePanel->getSize(),
                                showingScreen->geometry().height()));

        KWindowSystem::setExtendedStrut(this->winId(),
                                        this->m_profilePanel->getSize(),
                                        showingScreen->geometry().y(),
                                        showingScreen->geometry().y() + showingScreen->geometry().width(),
                                        0, 0, 0,
                                        0, 0, 0,
                                        0, 0, 0);
        break;
    default:
        // 默认放入底部
        this->setGeometry(QRect(showingScreen->geometry().x(),
                                showingScreen->geometry().bottom() - this->m_profilePanel->getSize(),
                                showingScreen->geometry().width(),
                                this->m_profilePanel->getSize()));

        KWindowSystem::setExtendedStrut(this->winId(),
                                        0, 0, 0,
                                        0, 0, 0,
                                        0, 0, 0,
                                        this->m_profilePanel->getSize(),
                                        showingScreen->geometry().x(),
                                        showingScreen->geometry().x() + showingScreen->geometry().height());
    }

    KLOG_DEBUG() << "Panel " << this->m_profilePanel->getUID() << "geometry: " << this->geometry();
}

void Panel::initChildren()
{
    auto direction = (this->m_orientation == PanelOrientation::PANEL_ORIENTATION_BOTTOM ||
                      this->m_orientation == PanelOrientation::PANEL_ORIENTATION_TOP)
                         ? QBoxLayout::Direction::LeftToRight
                         : QBoxLayout::Direction::TopToBottom;

    this->m_appletsLayout = new QBoxLayout(direction, this);
    this->m_appletsLayout->setContentsMargins(0, 0, 0, 0);
    this->m_appletsLayout->setSpacing(0);

    auto profileApplets = profile::getInstance()->getAppletsOnPanel(this->m_profilePanel->getUID());
    for (const auto& profileApplet : profileApplets)
    {
        auto applet = new Applet(profileApplet, this);
        this->m_appletsLayout->addWidget(applet);
    }
    this->m_appletsLayout->addStretch(0);
    KLOG_DEBUG() << this->m_appletsLayout->geometry();
}

int Panel::orientationStr2Enum(const QString& orientation)
{
    switch (shash(orientation.toStdString().c_str()))
    {
    case CONNECT("top", _hash):
        return PanelOrientation::PANEL_ORIENTATION_TOP;
    case CONNECT("right", _hash):
        return PanelOrientation::PANEL_ORIENTATION_RIGHT;
    case CONNECT("bottom", _hash):
        return PanelOrientation::PANEL_ORIENTATION_BOTTOM;
    case CONNECT("left", _hash):
        return PanelOrientation::PANEL_ORIENTATION_LEFT;
    default:
        return PanelOrientation::PANEL_ORIENTATION_BOTTOM;
    }
}
}  // namespace Kiran