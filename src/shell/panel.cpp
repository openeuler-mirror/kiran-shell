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
#include <KWindowSystem/KWindowSystem>
#include <QActionGroup>
#include <QApplication>
#include <QBoxLayout>
#include <QDesktopWidget>
#include <QFile>
#include <QMenu>
#include <QMouseEvent>
#include <QScopedPointer>
#include <QScreen>
#include <QSettings>

#include "applet.h"
#include "ks-config.h"
#include "lib/common/define.h"
#include "lib/common/setting-process.h"
#include "lib/common/utility.h"
#include "panel.h"
#include "profile/profile.h"
#include "utils.h"

namespace Kiran
{
Panel::Panel(ProfilePanel *profilePanel)
    : QWidget(nullptr, Qt::FramelessWindowHint),
      m_profilePanel(profilePanel)
{
    setAttribute(Qt::WA_X11NetWmWindowTypeDock);
    setAttribute(Qt::WA_TranslucentBackground);  // 透明

    init();
}

QString Panel::getUID()
{
    return m_profilePanel->getUID();
}

int Panel::getSize()
{
    return m_profilePanel->getSize();
}

int Panel::getOrientation()
{
    return int(this->orientationStr2Enum(m_profilePanel->getOrientation()));
}

void Panel::loadStylesheet(PaletteType paletteType)
{
    QString qssPath;
    switch (paletteType)
    {
    case Kiran::PALETTE_LIGHT:
        qssPath = ":/theme/themes/light_theme.qss";
        break;
    case Kiran::PALETTE_DARK:
        qssPath = ":/theme/themes/black_theme.qss";
        break;
    default:
        break;
    }
    QFile file(qssPath);
    if (file.open(QIODevice::ReadOnly))
    {
        QString style = file.readAll();
        this->setStyleSheet(style);
    }
    else
    {
        KLOG_INFO() << "load stylesheet failed";
    }
}

void Panel::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu0;

    {
        QMenu *menuLevel1 = menu0.addMenu(tr("Position"));
        QAction *actTop = menuLevel1->addAction(tr("Top"));
        QAction *actBottom = menuLevel1->addAction(tr("Bottom"));
        QAction *actLeft = menuLevel1->addAction(tr("Left"));
        QAction *actRight = menuLevel1->addAction(tr("Right"));
        actTop->setCheckable(true);
        actBottom->setCheckable(true);
        actLeft->setCheckable(true);
        actRight->setCheckable(true);

        QActionGroup *menuLevel1Group = new QActionGroup(&menu0);
        menuLevel1Group->addAction(actTop);
        menuLevel1Group->addAction(actRight);
        menuLevel1Group->addAction(actBottom);
        menuLevel1Group->addAction(actLeft);

        int orientation = getOrientation();
        if (orientation >= menuLevel1Group->actions().size())
        {
            menuLevel1Group->actions().at(menuLevel1Group->actions().size() - 1)->setChecked(true);
        }
        else
        {
            menuLevel1Group->actions().at(orientation)->setChecked(true);
        }

        connect(menuLevel1Group, &QActionGroup::triggered, this, [=](QAction *action)
                {
                    PanelOrientation orientation = (PanelOrientation)menuLevel1Group->actions().indexOf(action);
                    m_profilePanel->setOrientation(orientationEnum2Str(orientation));

                    if (orientation == PanelOrientation::PANEL_ORIENTATION_BOTTOM ||
                        orientation == PanelOrientation::PANEL_ORIENTATION_TOP)
                    {
                        m_appletsLayout->setContentsMargins(5, 0, 5, 0);
                    }
                    else
                    {
                        m_appletsLayout->setContentsMargins(0, 5, 0, 5);
                    }
                });
    }

    {
        QAction *act = menu0.addAction(tr("Show application name"));
        act->setCheckable(true);
        act->setChecked(SettingProcess::getValue(TASKBAR_SHOW_APP_BTN_TAIL_KEY).toBool());
        connect(act, &QAction::triggered, this, [=]()
                { SettingProcess::setValue(TASKBAR_SHOW_APP_BTN_TAIL_KEY, act->isChecked()); });
    }

    menu0.exec(mapToGlobal(event->pos()));
}

void Panel::init()
{
    //    loadStylesheet(Kiran::StylePalette::instance()->paletteType());
    //    QObject::connect(Kiran::StylePalette::instance(), &Kiran::StylePalette::themeChanged, this, &Panel::loadStylesheet);

    // 分辨率变化
    QObject::connect(getScreen(), &QScreen::geometryChanged, this, &Panel::updateShow);
    // 布局方向变化
    connect(m_profilePanel, &ProfilePanel::monitorChanged, this, &Panel::updateShow);
    connect(m_profilePanel, &ProfilePanel::sizeChanged, this, &Panel::updateShow);
    connect(m_profilePanel, &ProfilePanel::orientationChanged, this, &Panel::updateShow);

    this->initChildren();
    this->updateShow();
    this->show();
}

void Panel::initChildren()
{
    m_appletsLayout = new QBoxLayout(getLayoutDirection(), this);
    int orientation = getOrientation();
    if (orientation == PanelOrientation::PANEL_ORIENTATION_BOTTOM ||
        orientation == PanelOrientation::PANEL_ORIENTATION_TOP)
    {
        m_appletsLayout->setContentsMargins(5, 0, 5, 0);
    }
    else
    {
        m_appletsLayout->setContentsMargins(0, 5, 0, 5);
    }

    m_appletsLayout->setSpacing(5);

    auto profileApplets = Profile::getInstance()->getAppletsOnPanel(m_profilePanel->getUID());

    for (const auto &profileApplet : profileApplets)
    {
        auto applet = new Applet(profileApplet, this);

        m_appletsLayout->addWidget(applet);
    }
    //    m_appletsLayout->addStretch(0);
    KLOG_DEBUG() << m_appletsLayout->geometry();
}

int Panel::orientationStr2Enum(const QString &orientation)
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

QString Panel::orientationEnum2Str(const int &orientation)
{
    switch (orientation)
    {
    case PanelOrientation::PANEL_ORIENTATION_TOP:
        return "top";
    case PanelOrientation::PANEL_ORIENTATION_RIGHT:
        return "right";
    case PanelOrientation::PANEL_ORIENTATION_BOTTOM:
        return "bottom";
    case PanelOrientation::PANEL_ORIENTATION_LEFT:
        return "left";
    default:
        return "bottom";
        ;
    }
}

QScreen *Panel::getScreen()
{
    auto monitorIndex = m_profilePanel->getMonitor();

    auto screens = QGuiApplication::screens();
    QScreen *showingScreen = QApplication::primaryScreen();

    if (monitorIndex > 0 && monitorIndex < screens.size())
    {
        showingScreen = screens.at(monitorIndex);
    }
    else
    {
        //        KLOG_WARNING("The monitor index exceeds the maximum number of screens, so it will use primary screen.");
    }

    return showingScreen;
}

void Panel::updateShow()
{
    updateGeometry();
    updateLayout();

    //    KLOG_INFO() << "Panel sigPanelProfileChanged";

    //通知插件更新布局
    emit panelProfileChanged();
}

void Panel::updateGeometry()
{
    QScreen *showingScreen = getScreen();
    int orientation = getOrientation();

    //    KLOG_INFO() << "orientation: " << orientation
    //                << "screen geometry: " << showingScreen->geometry()
    //                << "panel size: " << getSize();

    int panelSize = getSize();  //宽或高
    QRect rect;
    switch (orientation)
    {
    case PanelOrientation::PANEL_ORIENTATION_TOP:
        rect = QRect(showingScreen->geometry().x(),
                     showingScreen->geometry().y(),
                     showingScreen->geometry().width(),
                     panelSize);
        break;
    case PanelOrientation::PANEL_ORIENTATION_RIGHT:
        rect = QRect(showingScreen->geometry().right() - panelSize,
                     showingScreen->geometry().y(),
                     panelSize,
                     showingScreen->geometry().height());
        break;
    case PanelOrientation::PANEL_ORIENTATION_LEFT:
        rect = QRect(showingScreen->geometry().x(),
                     showingScreen->geometry().y(),
                     panelSize,
                     showingScreen->geometry().height());
        break;
    default:
        // 默认放入底部
        rect = QRect(showingScreen->geometry().x(),
                     showingScreen->geometry().bottom() - panelSize,
                     showingScreen->geometry().width(),
                     panelSize);
    }

    KLOG_INFO() << "panel geometry:" << rect;
    //    setGeometry(rect);
    move(rect.topLeft());
    setMinimumSize(rect.size());
    setMaximumSize(rect.size());

    // 计算放置的位置，并且确保该区域不被其他窗口覆盖
    switch (orientation)
    {
    case PanelOrientation::PANEL_ORIENTATION_TOP:
        KWindowSystem::setExtendedStrut(this->winId(),
                                        0,
                                        0,
                                        0,
                                        0,
                                        0,
                                        0,
                                        panelSize,
                                        rect.left(),
                                        rect.right(),
                                        0,
                                        0,
                                        0);

        break;
    case PanelOrientation::PANEL_ORIENTATION_RIGHT:
        KWindowSystem::setExtendedStrut(this->winId(),
                                        0,
                                        0,
                                        0,
                                        panelSize,
                                        rect.top(),
                                        rect.bottom(),
                                        0,
                                        0,
                                        0,
                                        0,
                                        0,
                                        0);
        break;
    case PanelOrientation::PANEL_ORIENTATION_LEFT:
        KWindowSystem::setExtendedStrut(this->winId(),
                                        panelSize,
                                        rect.top(),
                                        rect.bottom(),
                                        0,
                                        0,
                                        0,
                                        0,
                                        0,
                                        0,
                                        0,
                                        0,
                                        0);
        break;
    default:
        // 默认放入底部
        KWindowSystem::setExtendedStrut(this->winId(),
                                        0,
                                        0,
                                        0,
                                        0,
                                        0,
                                        0,
                                        0,
                                        0,
                                        0,
                                        panelSize,
                                        rect.left(),
                                        rect.right());
    }

    KLOG_DEBUG() << "Panel " << m_profilePanel->getUID() << "geometry: " << this->geometry();
}

void Panel::updateLayout()
{
    m_appletsLayout->setDirection(getLayoutDirection());
}

QBoxLayout::Direction Panel::getLayoutDirection()
{
    int orientation = getOrientation();
    auto direction = (orientation == PanelOrientation::PANEL_ORIENTATION_BOTTOM ||
                      orientation == PanelOrientation::PANEL_ORIENTATION_TOP)
                         ? QBoxLayout::Direction::LeftToRight
                         : QBoxLayout::Direction::TopToBottom;
    return direction;
}

}  // namespace Kiran
