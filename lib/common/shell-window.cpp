/**
 * Copyright (c) 2026 KylinSec Co., Ltd.
 * kiran-shell is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     Xinhao Liu <liuxinhao@kylinsec.com.cn>
 */

#include "shell-window.h"

#include <KWindowSystem>
#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/plasmashell.h>
#include <KWayland/Client/registry.h>
#include <KWayland/Client/surface.h>

#include <QGuiApplication>
#include <QMoveEvent>
#include <QPlatformSurfaceEvent>
#include <QPointer>
#include <QTimer>
#include <QWindow>

#include "logging-category.h"

namespace Kiran
{

// Wayland 连接状态，所有 ShellWindow 实例共享
KWayland::Client::ConnectionThread *ShellWindow::s_connection = nullptr;
KWayland::Client::Registry *ShellWindow::s_registry = nullptr;
KWayland::Client::PlasmaShell *ShellWindow::s_plasmaShell = nullptr;
bool ShellWindow::s_waylandInitialized = false;

Qt::WindowFlags ShellWindow::suggestedFlags(ShellWindowRole role) noexcept
{
    if (role == ShellWindowRole::Panel) {
        return Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint;
    }
    // AppletPopup — 需要 Popup 标志来抓取鼠标和感知外部点击
    return Qt::FramelessWindowHint | Qt::Popup;
}

ShellWindow::ShellWindow(ShellWindowRole role, QWidget *parent)
    : QWidget(parent, suggestedFlags(role)),
      m_role(role),
      m_wayland(KWindowSystem::isPlatformWayland())
{
    if (m_wayland) {
        installEventFilter(this);
        winId();
    }

    if (!m_wayland && m_role == ShellWindowRole::Panel) {
        winId();
        KWindowSystem::setType(winId(), NET::Dock);
        KWindowSystem::setOnAllDesktops(winId(), true);
    }
}

ShellWindow::~ShellWindow()
{
    destroyShellSurface();
}

bool ShellWindow::event(QEvent *event)
{
    if (event->type() == QEvent::WinIdChange && !m_wayland && m_role == ShellWindowRole::Panel)
    {
        if (effectiveWinId() != 0)
        {
            KWindowSystem::setType(effectiveWinId(), NET::Dock);
            KWindowSystem::setOnAllDesktops(effectiveWinId(), true);
        }
    }
    return QWidget::event(event);
}

void ShellWindow::setPosition(const QPoint &pos)
{
    m_pendingPosition = pos;
    QWidget::move(pos);

    if (m_wayland && !m_shellSurface) {
        syncShellSurface();
    } else if (m_shellSurface) {
        m_shellSurface->setPosition(pos);
    }
}

void ShellWindow::setPosition(int x, int y)
{
    setPosition(QPoint(x, y));
}

void ShellWindow::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (m_wayland) {
        syncShellSurface();
    }
}

void ShellWindow::hideEvent(QHideEvent *event)
{
    if (m_wayland) {
        destroyShellSurface();
    }
    QWidget::hideEvent(event);
}

void ShellWindow::syncShellSurface()
{
    createShellSurface();
    if (m_shellSurface) {
        m_shellSurface->setPosition(m_pendingPosition);
    }
}

bool ShellWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (!m_wayland)
        return QWidget::eventFilter(watched, event);

    switch (event->type())
    {
    case QEvent::PlatformSurface:
        handlePlatformSurfaceEvent(static_cast<QPlatformSurfaceEvent *>(event));
        break;
    case QEvent::Expose:
        if (isVisible() && !m_shellSurface) {
            syncShellSurface();
        }
        break;
    case QEvent::Move:
        handleMoveEvent(static_cast<QMoveEvent *>(event));
        break;
    default:
        break;
    }

    return QWidget::eventFilter(watched, event);
}

void ShellWindow::handlePlatformSurfaceEvent(QPlatformSurfaceEvent *event)
{
    if (event->surfaceEventType() == QPlatformSurfaceEvent::SurfaceCreated) {
        KLOG_DEBUG(LCLib) << "ShellWindow: PlatformSurface created, role=" << static_cast<int>(m_role);
        if (m_shellSurface) {
            destroyShellSurface();
        }
        createShellSurface();
    } else if (event->surfaceEventType() == QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed) {
        KLOG_DEBUG(LCLib) << "ShellWindow: PlatformSurface destroying, role=" << static_cast<int>(m_role);
        destroyShellSurface();
    }
}

void ShellWindow::handleMoveEvent(QMoveEvent *event)
{
    m_pendingPosition = event->pos();
    if (m_shellSurface) {
        m_shellSurface->setPosition(event->pos());
    }
}

void ShellWindow::ensureWaylandConnection()
{
    if (s_waylandInitialized)
        return;

    if (!m_wayland)
        return;

    using namespace KWayland::Client;

    s_connection = ConnectionThread::fromApplication(nullptr);
    if (!s_connection) {
        KLOG_WARNING(LCLib) << "ShellWindow::ensureWaylandConnection failed: no Wayland connection";
        return;
    }

    s_registry = new Registry(nullptr);
    s_registry->create(s_connection);

    QObject::connect(s_registry, &Registry::plasmaShellAnnounced, [](quint32 name, quint32 version) {
        if (!s_plasmaShell) {
            s_plasmaShell = s_registry->createPlasmaShell(name, version, nullptr);
        }
    });

    s_registry->setup();
    s_connection->roundtrip();

    if (!s_plasmaShell) {
        KLOG_WARNING(LCLib) << "ShellWindow::ensureWaylandConnection failed: plasma shell not announced";
    }

    s_waylandInitialized = true;
}

void ShellWindow::createShellSurface()
{
    if (!m_wayland)
        return;

    if (m_shellSurface)
        return;

    ensureWaylandConnection();

    if (!s_connection || !s_plasmaShell)
        return;

    if (!windowHandle())
        return;

    auto *surface = KWayland::Client::Surface::fromWindow(windowHandle());
    if (!surface) {
        if (isVisible() && windowHandle()) {
            QPointer<ShellWindow> guard(this);
            QTimer::singleShot(80, this, [guard]() {
                if (guard && guard->isVisible()) {
                    guard->createShellSurface();
                }
            });
        }
        return;
    }

    m_shellSurface = s_plasmaShell->createSurface(surface, this);
    if (!m_shellSurface)
        return;

    if (m_role == ShellWindowRole::Panel) {
        m_shellSurface->setRole(KWayland::Client::PlasmaShellSurface::Role::Panel);
        m_shellSurface->setPanelBehavior(KWayland::Client::PlasmaShellSurface::PanelBehavior::AlwaysVisible);
    } else {
        m_shellSurface->setRole(KWayland::Client::PlasmaShellSurface::Role::AppletPopup);
    }

    m_shellSurface->setSkipTaskbar(true);
    m_shellSurface->setPosition(m_pendingPosition);
}

void ShellWindow::destroyShellSurface()
{
    if (m_shellSurface) {
        m_shellSurface->deleteLater();
        m_shellSurface = nullptr;
    }
}

}  // namespace Kiran
