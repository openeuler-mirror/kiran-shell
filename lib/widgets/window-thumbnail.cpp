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
#include <KWindowSystem>
#include <QMouseEvent>
#include <QPainter>

#include "lib/common/utility.h"
#include "lib/common/window-info-helper.h"
#include "lib/common/window-manager.h"
#include "ui_window-thumbnail.h"
#include "window-thumbnail.h"

namespace Kiran
{
WindowThumbnail::WindowThumbnail(WId wid, QWidget *parent)
    : QWidget(parent),
      m_ui(new Ui::WindowThumbnail),
      m_wid(wid),
      m_isHover(false)
{
    m_ui->setupUi(this);

    // 启用悬浮事件
    setAttribute(Qt::WA_Hover);

    // 标题栏图标
    QPixmap icon = KWindowSystem::icon(m_wid, 25, 25, true);
    m_ui->labelAppIcon->setPixmap(icon);
    updateVisualName();

    // FIXME:待主题开发好之后去掉
    QString style = "QPushButton{background:transparent; border:none; image:url(:/images/images/close_normal.png);} \
                    QPushButton:hover{image:url(:/images/images/close_hover.png);} \
                    QPushButton:pressed{image:url(:/images/images/close_pressed.png);}";
    m_ui->btnClose->setStyleSheet(style);
    m_ui->btnClose->hide();

    connect(&WindowManagerInstance, &Common::WindowManager::previewrUpdated, this, [this](WId wid)
            {
                if (wid == m_wid)
                {
                    refresh();
                }
            });

    connect(&WindowManagerInstance, &Common::WindowManager::windowChanged, this, &WindowThumbnail::changedWindow);

    refresh();
}

WindowThumbnail::~WindowThumbnail()
{
    delete m_ui;
}

void WindowThumbnail::showEvent(QShowEvent *event)
{
    refresh();

    QWidget::showEvent(event);
}

void WindowThumbnail::resizeEvent(QResizeEvent *event)
{
    refresh();
    updateVisualName();

    QWidget::resizeEvent(event);
}

void WindowThumbnail::mouseReleaseEvent(QMouseEvent *event)
{
    if (Qt::LeftButton == event->button())
    {
        WindowInfoHelper::activateWindow(m_wid);
    }

    QWidget::mouseReleaseEvent(event);
}

void WindowThumbnail::enterEvent(QEvent *event)
{
    m_isHover = true;
    m_ui->btnClose->show();

    update();  // 触发重绘
    QWidget::enterEvent(event);
}

void WindowThumbnail::leaveEvent(QEvent *event)
{
    m_isHover = false;
    m_ui->btnClose->hide();

    update();  // 触发重绘
    QWidget::leaveEvent(event);
}

void WindowThumbnail::paintEvent(QPaintEvent *event)
{
    // 默认绘制
    QWidget::paintEvent(event);

    if (m_isHover)
    {
        QPainter painter(this);
        auto palette = Kiran::Theme::Palette::getDefault();
        QPen pen;
        pen.setColor(palette->getColor(Kiran::Theme::Palette::SELECTED, Kiran::Theme::Palette::WIDGET));  // 鼠标悬浮时设置边框颜色
        pen.setWidth(4);                                                                                  // 边框宽度
        painter.setPen(pen);
        painter.drawRect(rect());  // 画出边框
    }
}

void WindowThumbnail::refresh()
{
    if (!isVisible())
    {
        return;
    }
    //    QPixmap pix = Kiran::Common::WindowManager::Instance().getPixPreviewr(m_wid);
    QPixmap pix = WindowManagerInstance.getPixPreviewr(m_wid);
    //    new Common::WindowManager();
    if (pix.isNull())
    {
        m_ui->labelGrabWindow->setPixmap(KWindowSystem::icon(m_wid, 60, 60, true));
    }
    else if (pix.size() != m_ui->labelGrabWindow->size())
    {
        pix = pix.scaled(m_ui->labelGrabWindow->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_ui->labelGrabWindow->setPixmap(pix);
    }
}

void WindowThumbnail::updateVisualName()
{
    QString visibleName = WindowInfoHelper::getAppNameByWId(m_wid);
    QFontMetrics fontMetrics = m_ui->labelAppName->fontMetrics();
    int elidedTextLen = m_ui->labelAppName->width();
    QString elideText = Utility::getElidedText(fontMetrics, visibleName, elidedTextLen);
    m_ui->labelAppName->setText(elideText);

    setToolTip(visibleName);
}

void WindowThumbnail::getOriginalSize(int &scaleWidth, int &scaleHeight, int &extraWidth, int &extraHeight)
{
    QSize originalSize = WindowManagerInstance.getPixPreviewr(m_wid).size();
    scaleWidth = originalSize.width();
    scaleHeight = originalSize.height();

    // 标题高度 控件间距 上下margin
    int addHeight = m_ui->labelAppName->height();
    addHeight += layout()->spacing();
    addHeight += layout()->contentsMargins().top() + layout()->contentsMargins().bottom();
    extraHeight = addHeight;

    int addWidth = layout()->contentsMargins().left() + layout()->contentsMargins().right();
    extraWidth = addWidth;
}

void WindowThumbnail::on_btnClose_clicked()
{
    WindowInfoHelper::closeWindow(m_wid);
}

void WindowThumbnail::changedWindow(WId wid, NET::Properties properties, NET::Properties2 properties2)
{
    if (m_wid != wid)
    {
        return;
    }

    if (properties.testFlag(NET::WMName))
    {
        updateVisualName();
    }
}

}  // namespace Kiran
