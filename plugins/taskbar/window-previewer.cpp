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
 * Author:     yangfeng <yangfeng@kylinsec.com.cn>
 */

#include <plugin-i.h>
#include <qt5-log-i.h>
#include <KWindowSystem>
#include <QGuiApplication>
#include <QMenu>
#include <QMouseEvent>
#include <QScreen>
#include <QTimer>

#include "app-previewer.h"
#include "lib/common/utility.h"
#include "lib/common/window-info-helper.h"
#include "ui_window-previewer.h"
#include "window-previewer.h"

namespace Kiran
{
namespace Taskbar
{
WindowPreviewer::WindowPreviewer(WId wid, IAppletImport *import, AppPreviewer *parent)
    : QWidget(parent),
      m_ui(new Ui::WindowPreviewer),
      m_wid(wid),
      m_import(import),
      m_updateInProgress(false)
{
    m_ui->setupUi(this);

    int panelSize = m_import->getPanel()->getSize();
    setFixedSize(panelSize * 4, panelSize * 4);

    // 标题栏图标
    QPixmap icon = KWindowSystem::icon(m_wid, 25, 25, true);
    m_ui->m_labelAppIcon->setPixmap(icon);

    m_menu = new QMenu(this);
    // 菜单弹出时，点击其地方，隐藏预览窗口
    connect(m_menu, &QMenu::aboutToHide, this, &WindowPreviewer::hideWindow);

    // FIXME:待主题开发好之后去掉
    QString style = "QPushButton{background:transparent; border:none; image:url(:/images/images/close_normal.png);} \
                    QPushButton:hover{image:url(:/images/images/close_hover.png);} \
                    QPushButton:pressed{image:url(:/images/images/close_pressed.png);}";
    m_ui->m_btnClose->setStyleSheet(style);

    connect(parent, &AppPreviewer::windowChanged, this, &WindowPreviewer::changedWindow);
    connect(parent, &AppPreviewer::activeWindowChanged, this, &WindowPreviewer::changedActiveWindow);
}

WindowPreviewer::~WindowPreviewer()
{
    delete m_ui;
}

void WindowPreviewer::updatePreviewer()
{
    //    KLOG_INFO() << "updatePreviewer" << m_wid;
    QScreen *screen = QGuiApplication::primaryScreen();
    m_pixPreviewer = screen->grabWindow(m_wid);
    if (m_pixPreviewer.isNull())
    {
        m_pixPreviewer = KWindowSystem::icon(m_wid);
    }

    m_updateInProgress = false;
}

bool WindowPreviewer::checkCanHide()
{
    return m_menu->isHidden();
}

void WindowPreviewer::changedWindow(WId wid, NET::Properties properties, NET::Properties2 properties2)
{
    if (m_wid == wid)
    {
        // 窗口位置变化
        // KLOG_INFO() << "WindowPreviewer::changedWindow" << properties << properties2;
        if (properties.testFlag(NET::WMGeometry))
        {
            startUpdatePreviewer();
        }
    }
}

void WindowPreviewer::changedActiveWindow(WId wid)
{
    if (wid == m_wid)
    {
        startUpdatePreviewer();
    }

    m_widLastActive = wid;
}

void WindowPreviewer::startUpdatePreviewer()
{
    if (!m_updateInProgress)
    {
        // 需要延迟处理：标志比窗口更新快，当标志完成更新时，窗口还没有更新完成
        // 避免短时间内多次调用
        m_updateInProgress = true;
        QTimer::singleShot(200, this, [this]()
                           { updatePreviewer(); });
    }
}

void WindowPreviewer::on_m_btnClose_clicked()
{
    emit closeWindow(m_wid);
}

void WindowPreviewer::mousePressEvent(QMouseEvent *event)
{
    if (Qt::LeftButton == event->button())
    {
        emit hideWindow();

        if (m_widLastActive != m_wid)
        {
            WindowInfoHelper::activateWindow(m_wid);
        }
        else
        {
            WindowInfoHelper::minimizeWindow(m_wid);
        }
    }

    QWidget::mousePressEvent(event);
}

void WindowPreviewer::contextMenuEvent(QContextMenuEvent *event)
{
    m_menu->clear();

    m_menu->addAction(tr("Close window"), this, [=]()
                      {
                          emit hideWindow();
                          emit closeWindow(m_wid);
                          WindowInfoHelper::activateWindow(m_wid);
                      });

    if (WindowInfoHelper::isMaximized(m_wid))
    {
        m_menu->addAction(tr("Restore"), this, [=]()
                          {
                              emit hideWindow();
                              WindowInfoHelper::maximizeWindow(m_wid, false);
                          });
    }
    else
    {
        m_menu->addAction(tr("Maximize"), this, [=]()
                          {
                              emit hideWindow();
                              WindowInfoHelper::maximizeWindow(m_wid, true);
                          });
    }

    if (!WindowInfoHelper::isMinimized(m_wid))
    {
        m_menu->addAction(tr("Minimize"), this, [=]()
                          {
                              emit hideWindow();
                              WindowInfoHelper::minimizeWindow(m_wid);
                          });
    }

    if (WindowInfoHelper::isKeepAboved(m_wid))
    {
        m_menu->addAction(tr("Do not keep above"), this, [=]()
                          {
                              emit hideWindow();
                              WindowInfoHelper::setKeepAbove(m_wid, false);
                          });
    }
    else
    {
        m_menu->addAction(tr("Keep above"), this, [=]()
                          {
                              emit hideWindow();
                              WindowInfoHelper::setKeepAbove(m_wid, true);
                          });
    }

    m_menu->exec(mapToGlobal(event->pos()));

    // 在这里发信号会导致窗口无法激活
    //    emit hideWindow();
    //    ((QWidget*)parent())->hide();
}

void WindowPreviewer::showEvent(QShowEvent *event)
{
    QString visibleName = WindowInfoHelper::getAppNameByWId(m_wid);
    QFontMetrics fontMetrics = m_ui->m_labelAppName->fontMetrics();
    int elidedTextLen = m_ui->m_labelAppName->width();
    QString elideText = Utility::getElidedText(fontMetrics, visibleName, elidedTextLen);
    m_ui->m_labelAppName->setText(elideText);

    if (m_pixPreviewer.isNull())
    {
        m_ui->m_labelGrabWindow->setPixmap(KWindowSystem::icon(m_wid, 60, 60, true));
    }
    else if (m_pixPreviewer.size() != m_ui->m_labelGrabWindow->size())
    {
        m_pixPreviewer = m_pixPreviewer.scaled(m_ui->m_labelGrabWindow->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_ui->m_labelGrabWindow->setPixmap(m_pixPreviewer);
    }
}

}  // namespace Taskbar

}  // namespace Kiran
