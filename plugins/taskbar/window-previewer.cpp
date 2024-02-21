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

#include <qt5-log-i.h>
#include <KWindowSystem>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QScreen>

#include "lib/common/window-info-helper.h"
#include "ui_window-previewer.h"
#include "window-previewer.h"

namespace Kiran
{
namespace Taskbar
{
WindowPreviewer::WindowPreviewer(WId wid, QWidget *parent)
    : QWidget(parent),
      m_ui(new Ui::WindowPreviewer),
      m_wid(wid)
{
    m_ui->setupUi(this);

    int iconSize = m_ui->m_labelAppIcon->width();
    QPixmap icon = KWindowSystem::icon(wid, iconSize, iconSize, true);
    m_ui->m_labelAppIcon->setPixmap(icon);

    QString name = WindowInfoHelper::getAppNameByWId(wid);
    m_ui->m_labelAppName->setText(name);

    m_menu = new QMenu(this);
    // 菜单弹出时，点击其地方，隐藏预览窗口
    connect(m_menu, &QMenu::aboutToHide, this, &WindowPreviewer::hideWindow);
}

WindowPreviewer::~WindowPreviewer()
{
    delete m_ui;
}

void WindowPreviewer::updatePreviewer()
{
    // FIXME:截取不到最小化窗口或被覆盖窗口
    QScreen *screen = QGuiApplication::primaryScreen();
    QPixmap screenshot = screen->grabWindow(m_wid);
    if (screenshot.isNull())
    {
        int screenshotSize = m_ui->m_labelGrabWindow->width();
        screenshot = KWindowSystem::icon(m_wid, screenshotSize, screenshotSize, true);
    }

    screenshot = screenshot.scaled(m_ui->m_labelGrabWindow->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_ui->m_labelGrabWindow->setPixmap(screenshot);
}

bool WindowPreviewer::checkCanHide()
{
    return m_menu->isHidden();
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
            WindowInfoHelper::activeWindow(m_wid);
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
                          WindowInfoHelper::activeWindow(m_wid);
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
    m_widLastActive = KWindowSystem::activeWindow();
}

}  // namespace Taskbar

}  // namespace Kiran
