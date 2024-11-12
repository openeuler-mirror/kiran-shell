/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
 * kiran-shell is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan
 * PSL v2. You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 * NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE. See the
 * Mulan PSL v2 for more details.
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#include <kiran-integration/theme/palette.h>
#include <ks-i.h>
#include <qt5-log-i.h>
#include <KWindowSystem/KWindowSystem>
#include <QActionGroup>
#include <QApplication>
#include <QBoxLayout>
#include <QDesktopWidget>
#include <QFile>
#include <QFrame>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
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

#define PERSONALITY_MODE_LAYOUT_MARGIN 4
#define PERSONALITY_MODE_RADIUS 4

#define SHELL_SCHEMA_ID "com.kylinsec.kiran.shell"
#define SHELL_SCHEMA_KEY_PERSONALITY_MODE "enablePersonalityMode"

namespace Kiran
{
Panel::Panel(ProfilePanel *profilePanel)
    : QWidget(nullptr, Qt::FramelessWindowHint),
      m_profilePanel(profilePanel),
      m_shellGsettings(nullptr),
      m_isPersonalityMode(false),
      m_layoutMargin(0),
      m_radius(0)
{
    setAttribute(Qt::WA_X11NetWmWindowTypeDock);
    setAttribute(Qt::WA_TranslucentBackground, true);  // 透明

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
            menuLevel1Group->actions()
                .at(menuLevel1Group->actions().size() - 1)
                ->setChecked(true);
        }
        else
        {
            menuLevel1Group->actions().at(orientation)->setChecked(true);
        }

        connect(menuLevel1Group, &QActionGroup::triggered, this,
                [=](QAction *action)
                {
                    PanelOrientation orientation =
                        (PanelOrientation)menuLevel1Group->actions().indexOf(action);
                    m_profilePanel->setOrientation(orientationEnum2Str(orientation));
                });
    }

    {
        QAction *act = menu0.addAction(tr("Show application name"));
        act->setCheckable(true);
        act->setChecked(
            SettingProcess::getValue(TASKBAR_SHOW_APP_BTN_TAIL_KEY).toBool());
        connect(act, &QAction::triggered, this, [=]()
                {
                    SettingProcess::setValue(TASKBAR_SHOW_APP_BTN_TAIL_KEY, act->isChecked());
                });
    }

    menu0.exec(mapToGlobal(event->pos()));
}

void Panel::paintEvent(QPaintEvent *event)
{
    auto palette = Kiran::Theme::Palette::getDefault();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);  // 设置反走样，使边缘平滑

    QColor bgColor = palette->getBaseColors().baseBackground;
    bgColor = palette->getColor(Kiran::Theme::Palette::ACTIVE,
                                Kiran::Theme::Palette::WINDOW);

    painter.setPen(Qt::NoPen);
    painter.setBrush(bgColor);

    QRectF rect = this->rect();

    if (!m_isPersonalityMode)
    {
        // 非个性模式，直接绘制背景
        painter.drawRect(rect);
    }
    else
    {
        // 个性模式，绘制头尾圆角
        int orientation = getOrientation();
        if (orientation == PanelOrientation::PANEL_ORIENTATION_BOTTOM ||
            orientation == PanelOrientation::PANEL_ORIENTATION_TOP)
        {
            rect.adjust(m_radius, m_radius, -width() + 100, -m_radius);
            painter.drawRoundedRect(rect, m_radius, m_radius);

            rect = this->rect();
            rect.adjust(width() - 100, m_radius, -m_radius, -m_radius);
            painter.drawRoundedRect(rect, m_radius, m_radius);
        }
        else
        {
            rect.adjust(m_radius, m_radius, -m_radius, -height() + 100);
            painter.drawRoundedRect(rect, m_radius, m_radius);

            rect = this->rect();
            rect.adjust(m_radius, height() - 100, -m_radius, -m_radius);
            painter.drawRoundedRect(rect, m_radius, m_radius);
        }
    }

    QWidget::paintEvent(event);
}

void Panel::init()
{
    // 分辨率变化
    QObject::connect(getScreen(), &QScreen::geometryChanged, this,
                     &Panel::updateLayout);
    // 布局方向变化
    connect(m_profilePanel, &ProfilePanel::monitorChanged, this,
            &Panel::updateLayout);
    connect(m_profilePanel, &ProfilePanel::sizeChanged, this,
            &Panel::updateLayout);
    connect(m_profilePanel, &ProfilePanel::orientationChanged, this,
            &Panel::updateLayout);

    try
    {
        m_shellGsettings = new QGSettings(SHELL_SCHEMA_ID);
    }
    catch (...)
    {
        KLOG_WARNING() << "new QGSettings failed:" << SHELL_SCHEMA_ID;
    }
    if (m_shellGsettings)
    {
        connect(m_shellGsettings, &QGSettings::changed, this, &Panel::shellSettingChanged);
    }

    initChildren();

    updatePersonalityMode();

    updateLayout();
    show();
}

void Panel::initChildren()
{
    m_appletsLayout = new QBoxLayout(getLayoutDirection(), this);
    m_appletsLayout->setSpacing(0);

    auto profileApplets =
        Profile::getInstance()->getAppletsOnPanel(m_profilePanel->getUID());

    for (int i = 0; i < profileApplets.size(); i++)
    {
        auto profileApplet = profileApplets.at(i);
        auto applet = new Applet(profileApplet, this);

        m_appletsLayout->addWidget(applet);
        m_applets.append(applet);

        if (i != profileApplets.size() - 1)
        {
            QFrame *line = new QFrame(this);
            line->setFrameShape(QFrame::VLine);
            m_appletsLayout->addWidget(line, 0, Qt::AlignCenter);
            m_lineFrame.append(line);
        }
    }

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
        //        KLOG_WARNING("The monitor index exceeds the maximum number of
        //        screens, so it will use primary screen.");
    }

    return showingScreen;
}

void Panel::updateGeometry()
{
    QScreen *showingScreen = getScreen();
    int orientation = getOrientation();

    //    KLOG_INFO() << "orientation: " << orientation
    //                << "screen geometry: " << showingScreen->geometry()
    //                << "panel size: " << getSize();

    int panelSize = getSize() + m_layoutMargin * 2;  // 宽或高
    QRect rect;
    switch (orientation)
    {
    case PanelOrientation::PANEL_ORIENTATION_TOP:
        rect = QRect(showingScreen->geometry().x(), showingScreen->geometry().y(),
                     showingScreen->geometry().width(), panelSize);
        break;
    case PanelOrientation::PANEL_ORIENTATION_RIGHT:
        rect = QRect(showingScreen->geometry().right() - panelSize,
                     showingScreen->geometry().y(), panelSize,
                     showingScreen->geometry().height());
        break;
    case PanelOrientation::PANEL_ORIENTATION_LEFT:
        rect = QRect(showingScreen->geometry().x(), showingScreen->geometry().y(),
                     panelSize, showingScreen->geometry().height());
        break;
    default:
        // 默认放入底部
        rect = QRect(showingScreen->geometry().x(),
                     showingScreen->geometry().bottom() - panelSize,
                     showingScreen->geometry().width(), panelSize);
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
        KWindowSystem::setExtendedStrut(this->winId(), 0, 0, 0, 0, 0, 0, panelSize,
                                        rect.left(), rect.right(), 0, 0, 0);

        break;
    case PanelOrientation::PANEL_ORIENTATION_RIGHT:
        KWindowSystem::setExtendedStrut(this->winId(), 0, 0, 0, panelSize,
                                        rect.top(), rect.bottom(), 0, 0, 0, 0, 0,
                                        0);
        break;
    case PanelOrientation::PANEL_ORIENTATION_LEFT:
        KWindowSystem::setExtendedStrut(this->winId(), panelSize, rect.top(),
                                        rect.bottom(), 0, 0, 0, 0, 0, 0, 0, 0, 0);
        break;
    default:
        // 默认放入底部
        KWindowSystem::setExtendedStrut(this->winId(), 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                        panelSize, rect.left(), rect.right());
    }

    KLOG_DEBUG() << "Panel " << m_profilePanel->getUID()
                 << "geometry: " << this->geometry();
}

void Panel::updateLayout()
{
    updateGeometry();

    m_appletsLayout->setDirection(getLayoutDirection());
    int orientation = getOrientation();
    if (orientation == PanelOrientation::PANEL_ORIENTATION_BOTTOM ||
        orientation == PanelOrientation::PANEL_ORIENTATION_TOP)
    {
        // 多留一个LAYOUT_MARGIN距离，用于绘制头尾圆角
        m_appletsLayout->setContentsMargins(m_layoutMargin + m_radius, 0,
                                            m_layoutMargin + m_radius, 0);
        for (auto line : m_lineFrame)
        {
            line->setFrameShape(QFrame::VLine);
            line->setFixedHeight(15);
        }
    }
    else
    {
        m_appletsLayout->setContentsMargins(0, m_layoutMargin + m_radius, 0,
                                            m_layoutMargin + m_radius);
        for (auto line : m_lineFrame)
        {
            line->setFrameShape(QFrame::HLine);
            line->setFixedWidth(15);
        }
    }

    m_appletsLayout->activate();

    // 通知插件更新布局
    emit panelProfileChanged();
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

void Panel::shellSettingChanged(const QString &key)
{
    if (SHELL_SCHEMA_KEY_PERSONALITY_MODE == key)
    {
        updatePersonalityMode();
        updateLayout();
    }
}

void Panel::updatePersonalityMode()
{
    m_isPersonalityMode = m_shellGsettings && m_shellGsettings->get(SHELL_SCHEMA_KEY_PERSONALITY_MODE).toBool();

    if (m_isPersonalityMode)
    {
        m_radius = PERSONALITY_MODE_RADIUS;
        m_layoutMargin = PERSONALITY_MODE_LAYOUT_MARGIN;
    }
    else
    {
        m_radius = 0;
        m_layoutMargin = 0;
    }

    for (int i = 0; i < m_applets.size(); i++)
    {
        auto applet = m_applets.at(i);
        bool isSpacer = "spacer" == applet->getID();

        if (isSpacer)
        {
            applet->setVisible(m_isPersonalityMode);
        }

        // PersonalityMode模式下, 两个插件间没有spacer时,需要显示分割线
        // 非PersonalityMode模式下, spacer前后分割线只显示一个,其他全显示
        if (i >= m_lineFrame.size())
        {
            continue;  // m_lineFrame相比m_applets少创建了一个
        }
        if (m_isPersonalityMode)
        {
            bool showLine = i + 1 < m_applets.size() &&
                            !isSpacer &&
                            "spacer" != m_applets.at(i + 1)->getID();
            m_lineFrame.at(i)->setVisible(showLine);
        }
        else
        {
            m_lineFrame.at(i)->setVisible(!isSpacer);
        }
    }
}

}  // namespace Kiran
