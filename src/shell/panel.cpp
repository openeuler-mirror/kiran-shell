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
#include <qt5-log-i.h>
#include <KWindowSystem/KWindowSystem>
#include <QActionGroup>
#include <QApplication>
#include <QBoxLayout>
#include <QContextMenuEvent>
#include <QEvent>
#include <QFrame>
#include <QGSettings>
#include <QPainter>
#include <QScreen>
#include <QTimer>

#include "applet.h"
#include "ks-config.h"
#include "ks-i.h"
#include "lib/common/desktop-helper.h"
#include "lib/common/logging-category.h"
#include "lib/common/utility.h"
#include "lib/common/window-info-helper.h"
#include "line-frame.h"
#include "panel.h"
#include "profile/profile.h"
#include "utils.h"

#define PERSONALITY_MODE_LAYOUT_MARGIN 4
#define PERSONALITY_MODE_RADIUS 4

namespace Kiran
{
Panel::Panel(ProfilePanel *profilePanel)
    : QWidget(nullptr),
      m_profilePanel(profilePanel)
{
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);  // 透明
                                                 //    setAttribute(Qt::WA_X11NetWmWindowTypeDock);
    KWindowSystem::setType(winId(), NET::Dock);
    KWindowSystem::setOnAllDesktops(winId(), true);
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
    return int(orientationStr2Enum(m_profilePanel->getOrientation()));
}

void Panel::paintEvent(QPaintEvent *event)
{
    auto palette = Kiran::Theme::Palette::getDefault();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);  // 设置反走样，使边缘平滑

    QColor bgColor = palette->getColor(Kiran::Theme::Palette::NORMAL,
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
        int randSize = 20;  // 圆角宽高
        int orientation = getOrientation();
        if (orientation == PanelOrientation::PANEL_ORIENTATION_BOTTOM ||
            orientation == PanelOrientation::PANEL_ORIENTATION_TOP)
        {
            rect.adjust(m_radius, m_radius, -width() + randSize, -m_radius);
            painter.drawRoundedRect(rect, m_radius, m_radius);

            rect = this->rect();
            rect.adjust(width() - randSize, m_radius, -m_radius, -m_radius);
            painter.drawRoundedRect(rect, m_radius, m_radius);
        }
        else
        {
            rect.adjust(m_radius, m_radius, -m_radius, -height() + randSize);
            painter.drawRoundedRect(rect, m_radius, m_radius);

            rect = this->rect();
            rect.adjust(m_radius, height() - randSize, -m_radius, -m_radius);
            painter.drawRoundedRect(rect, m_radius, m_radius);
        }
    }

    QWidget::paintEvent(event);
}

void Panel::enterEvent(QEvent *event)
{
    if (m_isAutoHide && !m_isFullShow)
    {
        m_leaveDetectTimer->stop();
        updateGeometry();  // 按配置的大小显示
    }

    QWidget::enterEvent(event);
}

void Panel::leaveEvent(QEvent *event)
{
    if (m_isAutoHide)
    {
        // 这里不能直接隐藏
        // 考虑情况如下：
        // 1. 右键菜单或子窗口显示也会触发 QEvent::Leave
        // 2. 鼠标从子窗口离开，不会触发这里的 QEvent::Leave
        // 处理措施：鼠标离开面板后，使用定时器，周期性检测鼠标是否还在面板或子控件上，如果不在了才隐藏面板
        m_leaveDetectTimer->start();
    }

    QWidget::leaveEvent(event);
}

void Panel::closeEvent(QCloseEvent *event)
{
    event->ignore();
}

void Panel::showEvent(QShowEvent *event)
{
    KLOG_INFO() << "Panel::showEvent";
    QWidget::showEvent(event);

    updateLayout();
}

bool Panel::event(QEvent *event)
{
    switch (event->type())
    {
    case QEvent::WinIdChange:
    {
        // Sometimes Qt needs to re-create the underlying window of the widget and
        // the winId() may be changed at runtime. So we need to reset all X11 properties
        // when this happens.
        if (effectiveWinId() == 0)
            break;
        KLOG_INFO() << "Panel::event QEvent::WinIdChange";
        KWindowSystem::setType(effectiveWinId(), NET::Dock);
        KWindowSystem::setOnAllDesktops(effectiveWinId(), true);
        updateLayout();
    }
    default:
        break;
    }

    return QWidget::event(event);
}

void Panel::init()
{
    // 主屏变化
    connect(qGuiApp, &QGuiApplication::primaryScreenChanged, this, [this]()
            {
                // 尝试重新连接到新的当前屏幕
                connectToCurrentScreen();
                updateLayout();
            });

    // 初始连接到当前屏幕
    connectToCurrentScreen();

    // 布局方向变化
    connect(m_profilePanel, &ProfilePanel::monitorChanged, this, [this]()
            {
                connectToCurrentScreen();  // 面板中显示器配置变化时重新连接
                updateLayout();
            });
    connect(m_profilePanel, &ProfilePanel::sizeChanged, this,
            &Panel::updateLayout);
    connect(m_profilePanel, &ProfilePanel::orientationChanged, this,
            &Panel::updateLayout);
    connect(Profile::getInstance(), &Profile::appletUIDsChanged, [this]()
            {
                initChildren();
                updateLayout();
            });

    connect(&DesktopHelperInstance, &DesktopHelper::currentDesktopChanged, this, &Panel::updateLayout);
    connect(&DesktopHelperInstance, &DesktopHelper::numberOfDesktopsChanged, this, &Panel::updateLayout);

    m_gsettings = new QGSettings(SHELL_SCHEMA_ID, "", this);
    connect(m_gsettings, &QGSettings::changed, this, &Panel::shellSettingChanged);

    initChildren();
    updateLayout();

    m_leaveDetectTimer = new QTimer(this);
    m_leaveDetectTimer->setInterval(500);
    connect(m_leaveDetectTimer, &QTimer::timeout, this, [this]()
            {
                // 检查鼠标是否仍在窗口或其子窗口上，处理右键菜单和子窗口
                bool result = isMouseInsideWidgetTree(this);
                if (!result)
                {
                    // 如果鼠标不在窗口内，则隐藏窗口或调整大小
                    updateGeometry(1);  // 显示一个像素

                    // 停止定时器
                    m_leaveDetectTimer->stop();
                }
            });

    show();
}

void Panel::initChildren()
{
    if (m_appletsLayout)
    {
        Utility::clearLayout(m_appletsLayout);
        while (m_lineFrames.size())
        {
            auto line = m_lineFrames.takeFirst();
            line->setParent(nullptr);
            line->hide();
            delete line;
        }
    }
    else
    {
        m_appletsLayout = new QBoxLayout(getLayoutDirection(), this);
        m_appletsLayout->setSpacing(0);
    }

    m_appletsUID.clear();

    // 新插件
    auto profileApplets =
        Profile::getInstance()->getAppletsOnPanel(m_profilePanel->getUID());
    QMap<QString, ProfileApplet *> profileAppletsWithID;
    for (auto profileApplet : profileApplets)
    {
        profileAppletsWithID.insert(profileApplet->getUID(), profileApplet);
    }

    // 构建插件
    QList<Applet *> appletsNew;
    for (int i = 0; i < profileApplets.size(); i++)
    {
        Applet *applet;
        auto profileApplet = profileApplets.at(i);
        QString profileAppletUID = profileApplet->getUID();
        if (!m_applets.contains(profileAppletUID))
        {
            applet = new Applet(profileApplet, this);
            m_applets.insert(profileAppletUID, applet);
        }
        else
        {
            applet = m_applets[profileAppletUID];
        }

        m_appletsLayout->addWidget(applet);
        m_appletsUID.append(profileAppletUID);

        if (i != profileApplets.size() - 1)
        {
            LineFrame *line = new LineFrame(this);
            line->setFrameShape(QFrame::VLine);
            m_appletsLayout->addWidget(line, 0, Qt::AlignCenter);
            m_lineFrames.append(line);
        }
    }

    // 清理已删除的插件
    for (auto uid : m_applets.keys())
    {
        if (!profileAppletsWithID.contains(uid))
        {
            delete m_applets.take(uid);
        }
    }
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
    QScreen *showingScreen = QGuiApplication::primaryScreen();

    /**
     * NOTE:
     * monitorIndex为-1，则使用primaryScreen, 主屏变化时，会自动更新
     * monitorIndex为有效显示器索引，则根据显示器索引走，如果索引超出范围，则使用primaryScreen
     * monitorIndex为其他异常值，则使用primaryScreen
     *
     * 当主屏幕变化时，未显式指定有效显示器索引的底部面板需更随主屏变化
     * 如果有多个屏幕显示底部面板的需求，使用monitorIndex指定有效显示器索引，去除monitorIndex为-1的默认配置
     */
    if (monitorIndex == -1)
    {
        showingScreen = QGuiApplication::primaryScreen();
    }
    else if (monitorIndex >= 0 && monitorIndex < screens.size())
    {
        showingScreen = screens.at(monitorIndex);
    }
    else
    {
        KLOG_WARNING(LCShell) << "The monitor index exceeds the maximum number of screens, so it will use primary screen.";
    }

    return showingScreen;
}

void Panel::updateGeometry(int size)
{
    int panelSize = size;
    m_isFullShow = false;
    if (0 == panelSize)
    {
        panelSize = getSize() + m_layoutMargin * 2;  // 宽或高
        m_isFullShow = true;
    }

    QScreen *showingScreen = getScreen();
    int orientation = getOrientation();

    KLOG_INFO(LCShell) << "desktop:" << DesktopHelper::currentDesktop() << "orientation: " << orientation
                       << "screen geometry: " << showingScreen->geometry()
                       << "panel size: " << getSize();

    QRect rect;
    switch (orientation)
    {
    case PanelOrientation::PANEL_ORIENTATION_TOP:
        rect = QRect(showingScreen->geometry().x(), showingScreen->geometry().y(),
                     showingScreen->geometry().width(), panelSize);
        break;
    case PanelOrientation::PANEL_ORIENTATION_RIGHT:
        rect = QRect(showingScreen->geometry().x() + showingScreen->geometry().width() - panelSize,
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
                     showingScreen->geometry().y() + showingScreen->geometry().height() - panelSize,
                     showingScreen->geometry().width(), panelSize);
    }

    KLOG_INFO(LCShell) << "panel geometry:" << rect;
    //    setGeometry(rect);
    move(rect.topLeft());
    setMinimumSize(rect.size());
    setMaximumSize(rect.size());

    // 计算放置的位置，并且确保该区域不被其他窗口覆盖
    switch (orientation)
    {
    case PanelOrientation::PANEL_ORIENTATION_TOP:
        KWindowSystem::setExtendedStrut(winId(), 0, 0, 0, 0, 0, 0, panelSize,
                                        rect.left(), rect.right(), 0, 0, 0);

        break;
    case PanelOrientation::PANEL_ORIENTATION_RIGHT:
        KWindowSystem::setExtendedStrut(winId(), 0, 0, 0, panelSize,
                                        rect.top(), rect.bottom(), 0, 0, 0, 0, 0,
                                        0);
        break;
    case PanelOrientation::PANEL_ORIENTATION_LEFT:
        KWindowSystem::setExtendedStrut(winId(), panelSize, rect.top(),
                                        rect.bottom(), 0, 0, 0, 0, 0, 0, 0, 0, 0);
        break;
    default:
        // 默认放入底部
        KWindowSystem::setExtendedStrut(winId(), 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                        panelSize, rect.left(), rect.right());
    }

    KLOG_DEBUG(LCShell) << "Panel " << m_profilePanel->getUID()
                        << "geometry: " << geometry();
}

void Panel::updateLayout()
{
    if (!m_appletsLayout)
    {
        return;
    }
    updatePersonalityMode();
    updateAutoHide();

    int panelSize = getSize();
    m_appletsLayout->setDirection(getLayoutDirection());
    int orientation = getOrientation();
    if (orientation == PanelOrientation::PANEL_ORIENTATION_BOTTOM ||
        orientation == PanelOrientation::PANEL_ORIENTATION_TOP)
    {
        // 多留一个LAYOUT_MARGIN距离，用于绘制头尾圆角
        m_appletsLayout->setContentsMargins(m_layoutMargin + m_radius, 0,
                                            m_layoutMargin + m_radius, 0);
        for (auto line : m_lineFrames)
        {
            line->setFrameShape(QFrame::VLine);
            line->setFixedHeight(panelSize);
            line->setFixedWidth(10);
        }
    }
    else
    {
        m_appletsLayout->setContentsMargins(0, m_layoutMargin + m_radius, 0,
                                            m_layoutMargin + m_radius);
        for (auto line : m_lineFrames)
        {
            line->setFrameShape(QFrame::HLine);
            line->setFixedHeight(10);
            line->setFixedWidth(panelSize);
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
        updateLayout();
        return;
    }

    if (SHELL_SCHEMA_KEY_AUTO_HIDE == key)
    {
        updateLayout();
        return;
    }
}

void Panel::updatePersonalityMode()
{
    m_isPersonalityMode = m_gsettings && m_gsettings->get(SHELL_SCHEMA_KEY_PERSONALITY_MODE).toBool();

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

    for (int i = 0; i < m_appletsUID.size(); i++)
    {
        auto applet = m_applets[m_appletsUID.at(i)];
        bool isSpacer = "spacer" == applet->getID();

        if (isSpacer)
        {
            applet->setVisible(m_isPersonalityMode);
        }

        // PersonalityMode模式下, 两个插件间没有spacer时,需要显示分割线
        // 非PersonalityMode模式下, spacer前后分割线只显示一个,其他全显示
        if (i >= m_lineFrames.size())
        {
            continue;  // m_lineFrame相比m_applets少创建了一个
        }
        if (m_isPersonalityMode)
        {
            bool showLine = i + 1 < m_appletsUID.size() &&
                            !isSpacer &&
                            "spacer" != m_applets[m_appletsUID.at(i + 1)]->getID();
            m_lineFrames.at(i)->setVisible(showLine);
        }
        else
        {
            m_lineFrames.at(i)->setVisible(!isSpacer);
        }
    }
}

void Panel::updateAutoHide()
{
    m_isAutoHide = m_gsettings && m_gsettings->get(SHELL_SCHEMA_KEY_AUTO_HIDE).toBool();
    updateGeometry(m_isAutoHide);  // 配置更新，初始状态：开启自动隐藏->显示一个像素（int)true = 1  未开启自动隐藏->按配置的大小显示（int)false = 0
}

bool Panel::isMouseInsideWidgetTree(QWidget *widget)
{
    // 获取当前控件的几何信息
    QRect globalRect = QRect(
        widget->mapToGlobal(widget->geometry().topLeft()),
        widget->size());
    // 判断当前控件的几何范围是否包含鼠标位置
    if ((widget->isVisible() && globalRect.contains(QCursor::pos())) ||
        // 激活了子窗口
        (widget != this && WindowInfoHelper::isActived(widget->winId())))
    {
        return true;  // 如果当前控件包含鼠标位置，则返回 true
    }

    // 遍历当前控件的所有子控件
    const auto children = widget->findChildren<QWidget *>();
    for (QWidget *childWidget : children)
    {
        // 对每个子控件进行递归检查
        if (isMouseInsideWidgetTree(childWidget))
        {
            return true;  // 如果某个子控件（或其子控件）包含鼠标位置，返回 true
        }
    }

    return false;  // 如果没有控件包含鼠标位置，返回 false
}

void Panel::connectToCurrentScreen()
{
    // 更新面板配置屏幕信息，重新连接感知屏幕大小位置变化
    if (m_screenConnection)
    {
        disconnect(m_screenConnection);
    }

    QScreen *currentScreen = getScreen();
    if (currentScreen)
    {
        m_screenConnection = connect(currentScreen, &QScreen::geometryChanged, this,
                                     &Panel::updateLayout);
        KLOG_DEBUG(LCShell) << getUID() << "Connected to screen geometry changes for screen: " << currentScreen->name();
    }
}

}  // namespace Kiran
