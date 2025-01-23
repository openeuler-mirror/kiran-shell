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
 * Author:     yangfeng <yangfeng@kylinsec.com.cn>
 */

#include <kiran-integration/theme/palette.h>
#include <qt5-log-i.h>
#include <KWindowSystem/NETWM>
#include <QGSettings>
#include <QPainter>
#include <QScreen>
#include <QtX11Extras/QX11Info>

#include "desktop-helper.h"
#include "ks-i.h"
#include "lib/common/logging-category.h"
#include "lib/common/window-info-helper.h"
#include "lib/common/window-manager.h"
#include "ui_workspace-thumbnail.h"
#include "workspace-thumbnail.h"

namespace Kiran
{
namespace Workspace
{
WorkspaceThumbnail::WorkspaceThumbnail(int desktop, QWidget* parent)
    : QWidget(parent),
      m_ui(new Ui::WorkspaceThumbnail),
      m_workspaceIndex(desktop),
      m_isHover(false)
{
    m_ui->setupUi(this);

    // 启用悬浮事件
    setAttribute(Qt::WA_Hover);

    // FIXME:待主题开发好之后去掉
    QString style = "QPushButton{background:transparent; border:none; image:url(:/images/images/close_normal.png);} \
                    QPushButton:hover{image:url(:/images/images/close_hover.png);} \
                    QPushButton:pressed{image:url(:/images/images/close_pressed.png);}";
    m_ui->btnClose->setStyleSheet(style);
    m_ui->btnClose->hide();

    m_ui->labelIndex->setText(QString::number(m_workspaceIndex));

    m_ui->labelGrabWorkspace->clear();

    getDesktopBackground();

    connect(&WindowManagerInstance, &Common::WindowManager::windowRemoved, this, [this]()
            {
                update();
            });
}

WorkspaceThumbnail::~WorkspaceThumbnail()
{
    delete m_ui;
}

void WorkspaceThumbnail::updatePreviewer()
{
    // FIXME: 如果使用kwin,可以通过kwin的dbus接口获取截图
    // QDBusInterface interface("org.kde.KWin", "/Screenshot", "org.kde.kwin.Screenshot");

    // 这种方式只有工作区显示时才能截图
    //    QScreen* screen = QGuiApplication::primaryScreen();
    //    if (screen)
    //    {
    //        QPixmap screenshot = screen->grabWindow(0);
    //        m_ui->labelGrabWorkspace->setPixmap(screenshot.scaledToWidth(230));
    //    }

    // 参考kiran-menu的方式,先获取各个窗口的位置截图,跟桌面壁纸组合成工作区截图
    // 见 paintEvent
}

void WorkspaceThumbnail::on_btnClose_clicked()
{
    emit removeDesktop(m_workspaceIndex);
}

void WorkspaceThumbnail::enterEvent(QEvent* event)
{
    m_isHover = true;
    m_ui->btnClose->show();

    update();  // 触发重绘
    QWidget::enterEvent(event);
}

void WorkspaceThumbnail::leaveEvent(QEvent* event)
{
    m_isHover = false;
    m_ui->btnClose->hide();

    update();  // 触发重绘
    QWidget::leaveEvent(event);
}

void WorkspaceThumbnail::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);

    QPainter painter;

    getDesktopBackground();
    QPixmap pixPaint = m_desktopBackground.copy();  // 桌面背景

    painter.begin(&pixPaint);

    // 叠加当前桌面的窗口预览图
    QList<WId> windows = WindowManagerInstance.getAllWindow(m_workspaceIndex);
    for (auto window : windows)
    {
        auto pix = WindowManagerInstance.getPixPreviewr(window);
        auto rect = WindowManagerInstance.getWindowGeometry(window);
        painter.drawPixmap(rect, pix);
    }
    painter.end();

    painter.begin(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    NETRootInfo ri(QX11Info::connection(), NET::DesktopGeometry);
    auto geo = ri.desktopGeometry();
    auto paintRect = m_ui->labelGrabWorkspace->geometry();
    painter.drawPixmap(paintRect, pixPaint, QRect(0, 0, geo.width, geo.height));
    painter.end();

    auto palette = Kiran::Theme::Palette::getDefault();

    if (m_workspaceIndex == DesktopHelper::currentDesktop())
    {
        painter.begin(this);
        QPen pen;
        pen.setColor(palette->getColor(Kiran::Theme::Palette::SELECTED, Kiran::Theme::Palette::WIDGET));  // 鼠标悬浮时设置边框颜色
        pen.setWidth(4);                                                                                  // 边框宽度
        painter.setPen(pen);
        painter.drawRect(rect());

        painter.end();
    }

    if (m_isHover)
    {
        // 绘制背景
        painter.begin(this);
        QColor color = palette->getColor(Kiran::Theme::Palette::SELECTED, Kiran::Theme::Palette::WIDGET);
        color.setAlpha(100);
        painter.setPen(Qt::NoPen);
        painter.setBrush(color);
        painter.drawRect(rect());  // 绘制背景
        painter.end();
    }
}

void WorkspaceThumbnail::getDesktopBackground()
{
    auto gsettings = QSharedPointer<QGSettings>(new QGSettings(APPEARANCE_SCHEMA_ID));
    if (gsettings.isNull())
    {
        KLOG_ERROR(LCWorkspace) << APPEARANCE_SCHEMA_ID << "QGSettings schema create failed";
        return;
    }
    auto desktopBackgroundValue = gsettings->get(APPEARANCE_SCHEMA_KEY_DESKTOP_BACKGROUND);
    if (desktopBackgroundValue.isNull() || !desktopBackgroundValue.isValid())
    {
        KLOG_ERROR(LCWorkspace) << APPEARANCE_SCHEMA_ID << APPEARANCE_SCHEMA_KEY_DESKTOP_BACKGROUND << "QGSettings key get failed";
        return;
    }
    auto desktopBackground = desktopBackgroundValue.toString();

    m_desktopBackground.load(desktopBackground);
}

}  // namespace Workspace
}  // namespace Kiran
