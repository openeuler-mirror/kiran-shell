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
#include <qt5-log-i.h>
#include <KWindowSystem/KWindowSystem>
#include <QApplication>
#include <QBoxLayout>
#include <QScopedPointer>
#include <QScreen>
#include "src/shell/applet.h"
#include "src/shell/layout.h"
#include "src/shell/plugin.h"
#include "src/shell/utils.h"

namespace Kiran
{
Panel::Panel(QSharedPointer<Model::Panel> panelModel) : QWidget(nullptr, Qt::FramelessWindowHint),
                                                        m_panelModel(panelModel)
{
    this->setAttribute(Qt::WA_X11NetWmWindowTypeDock);
    this->m_orientation = this->orientationStr2Enum(this->m_panelModel->getOrientation());

    this->initUI();
}

QString Panel::getUID()
{
    return this->m_panelModel->getUID();
}

void Panel::initUI()
{
    this->initSelf();
    this->initChildren();
}

void Panel::initSelf()
{
    // 获取所在显示器
    auto monitorIndex = this->m_panelModel->getMonitor();

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
                 << "height: " << this->m_panelModel->getSize();

    // 计算放置的位置，并且确保该区域不被其他窗口覆盖
    switch (this->m_orientation)
    {
    case Orientation::ORIENTATION_TOP:
        this->setGeometry(QRect(showingScreen->geometry().x(),
                                showingScreen->geometry().y(),
                                showingScreen->geometry().width(),
                                this->m_panelModel->getSize()));

        KWindowSystem::setExtendedStrut(this->winId(),
                                        0, 0, 0,
                                        0, 0, 0,
                                        this->m_panelModel->getSize(),
                                        showingScreen->geometry().x(),
                                        showingScreen->geometry().x() + showingScreen->geometry().width(),
                                        0, 0, 0);

        break;
    case Orientation::ORIENTATION_RIGHT:
        this->setGeometry(QRect(showingScreen->geometry().right() - this->m_panelModel->getSize(),
                                showingScreen->geometry().y(),
                                this->m_panelModel->getSize(),
                                showingScreen->geometry().height()));

        KWindowSystem::setExtendedStrut(this->winId(),
                                        0, 0, 0,
                                        this->m_panelModel->getSize(),
                                        showingScreen->geometry().y(),
                                        showingScreen->geometry().y() + showingScreen->geometry().height(),
                                        0, 0, 0,
                                        0, 0, 0);
        break;
    case Orientation::ORIENTATION_LEFT:
        this->setGeometry(QRect(showingScreen->geometry().x(),
                                showingScreen->geometry().y(),
                                this->m_panelModel->getSize(),
                                showingScreen->geometry().height()));

        KWindowSystem::setExtendedStrut(this->winId(),
                                        this->m_panelModel->getSize(),
                                        showingScreen->geometry().y(),
                                        showingScreen->geometry().y() + showingScreen->geometry().width(),
                                        0, 0, 0,
                                        0, 0, 0,
                                        0, 0, 0);
        break;
    default:
        // 默认放入底部
        this->setGeometry(QRect(showingScreen->geometry().x(),
                                showingScreen->geometry().bottom() - this->m_panelModel->getSize(),
                                showingScreen->geometry().width(),
                                this->m_panelModel->getSize()));

        KWindowSystem::setExtendedStrut(this->winId(),
                                        0, 0, 0,
                                        0, 0, 0,
                                        0, 0, 0,
                                        this->m_panelModel->getSize(),
                                        showingScreen->geometry().x(),
                                        showingScreen->geometry().x() + showingScreen->geometry().height());
    }

    KLOG_DEBUG() << "Panel " << this->m_panelModel->getUID() << "geometry: " << this->geometry();
}

void Panel::initChildren()
{
    auto direction = (this->m_orientation == Panel::Orientation::ORIENTATION_BOTTOM ||
                      this->m_orientation == Panel::Orientation::ORIENTATION_TOP)
                         ? QBoxLayout::Direction::LeftToRight
                         : QBoxLayout::Direction::TopToBottom;

    this->m_appletsLayout = new QBoxLayout(direction, this);
    this->m_appletsLayout->setContentsMargins(0, 0, 0, 0);

    auto appletsModel = Model::Layout::getInstance()->getAppletsOnPanel(this->m_panelModel->getUID());
    for (const auto& appletModel : appletsModel)
    {
        auto applet = this->createApplet(appletModel);
        if (applet)
        {
            this->m_appletsLayout->addSpacerItem(new QSpacerItem(500, 0, QSizePolicy::Fixed, QSizePolicy::Fixed));
            this->m_appletsLayout->addWidget(applet);
        }
        else
        {
            KLOG_WARNING() << "Cannot create a valid applet for " << appletModel->getUID() << ", so delete the applet.";
        }
    }
    KLOG_DEBUG() << this->m_appletsLayout->geometry();
}

QWidget* Panel::createApplet(QSharedPointer<Model::Applet> appletModel)
{
    auto appletID = appletModel->getID();
    auto plugin = PluginPool::getInstance()->findPluginForApplet(appletID);

    if (!plugin)
    {
        KLOG_WARNING() << "Not found plugin for applet id: " << appletID;
        return nullptr;
    }

    return plugin->createApplet(appletID);
}

Panel::Orientation Panel::orientationStr2Enum(const QString& orientation)
{
    switch (shash(orientation.toStdString().c_str()))
    {
    case CONNECT("top", _hash):
        return Panel::Orientation::ORIENTATION_TOP;
    case CONNECT("right", _hash):
        return Panel::Orientation::ORIENTATION_RIGHT;
    case CONNECT("bottom", _hash):
        return Panel::Orientation::ORIENTATION_BOTTOM;
    case CONNECT("left", _hash):
        return Panel::Orientation::ORIENTATION_LEFT;
    default:
        return Panel::Orientation::ORIENTATION_BOTTOM;
    }
}
}  // namespace Kiran