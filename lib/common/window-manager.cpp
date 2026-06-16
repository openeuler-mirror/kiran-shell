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

#include <KWindowSystem>

#include "lib/common/wayland-window-backend.h"
#include "lib/common/x11-window-backend.h"
#include "window-manager.h"

namespace Kiran
{
namespace Common
{
WindowManager::WindowManager()
{
    if (KWindowSystem::isPlatformX11())
    {
        m_backend.reset(new X11WindowBackend());
    }
    else
    {
        m_backend.reset(new WaylandWindowBackend());
    }

    connect(m_backend.data(), &WindowManagerBackend::windowAdded, this, &WindowManager::windowAdded);
    connect(m_backend.data(), &WindowManagerBackend::windowRemoved, this, &WindowManager::windowRemoved);
    connect(m_backend.data(), &WindowManagerBackend::activeWindowChanged, this, &WindowManager::activeWindowChanged);
    connect(m_backend.data(), &WindowManagerBackend::windowTitleChanged, this, &WindowManager::windowTitleChanged);
    connect(m_backend.data(), &WindowManagerBackend::windowIconChanged, this, &WindowManager::windowIconChanged);
    connect(m_backend.data(), &WindowManagerBackend::windowStateChanged, this, &WindowManager::windowStateChanged);
    connect(m_backend.data(), &WindowManagerBackend::windowGeometryChanged, this, &WindowManager::windowGeometryChanged);
    connect(m_backend.data(), &WindowManagerBackend::windowChanged,
            this, &WindowManager::windowChanged);
    connect(m_backend.data(), &WindowManagerBackend::windowDesktopChanged,
            this, &WindowManager::windowDesktopChanged);
    connect(m_backend.data(), &WindowManagerBackend::currentDesktopChanged, this, &WindowManager::currentDesktopChanged);
    connect(m_backend.data(), &WindowManagerBackend::numberOfDesktopsChanged, this, &WindowManager::numberOfDesktopsChanged);
}

WindowManager::~WindowManager() = default;

WindowManager& WindowManager::getInstance()
{
    static WindowManager instance;
    return instance;
}

QList<WId> WindowManager::getAllWindow()
{
    return m_backend->getAllWindows();
}

QList<WId> WindowManager::getAllWindow(int desktop)
{
    return m_backend->getAllWindows(desktop);
}

QRect WindowManager::getWindowGeometry(WId wid)
{
    return m_backend->getWindowGeometry(wid);
}

QPixmap WindowManager::getPixPreviewr(WId wid)
{
    return m_backend->getWindowPreview(wid);
}

QString WindowManager::getWindowAppId(WId wid) const
{
    return m_backend->getWindowAppId(wid);
}

QString WindowManager::getWindowTitle(WId wid) const
{
    return m_backend->getWindowTitle(wid);
}

QString WindowManager::getWindowIconName(WId wid) const
{
    return m_backend->getWindowIconName(wid);
}

QByteArray WindowManager::getWindowDesktopFileName(WId wid) const
{
    return m_backend->getWindowDesktopFileName(wid);
}

int WindowManager::getWindowPid(WId wid) const
{
    return m_backend->getWindowPid(wid);
}

bool WindowManager::isSkipTaskbar(WId wid) const
{
    return m_backend->isSkipTaskbar(wid);
}

bool WindowManager::isMinimized(WId wid) const
{
    return m_backend->isMinimized(wid);
}

bool WindowManager::isMaximized(WId wid) const
{
    return m_backend->isMaximized(wid);
}

bool WindowManager::isKeepAbove(WId wid) const
{
    return m_backend->isKeepAbove(wid);
}

bool WindowManager::isActive(WId wid) const
{
    return m_backend->isActive(wid);
}

WId WindowManager::activeWindow() const
{
    return m_backend->activeWindow();
}

void WindowManager::closeWindow(WId wid)
{
    m_backend->closeWindow(wid);
}

void WindowManager::activateWindow(WId wid)
{
    m_backend->activateWindow(wid);
}

void WindowManager::minimizeWindow(WId wid)
{
    m_backend->minimizeWindow(wid);
}

void WindowManager::maximizeWindow(WId wid, bool set)
{
    m_backend->maximizeWindow(wid, set);
}

void WindowManager::restoreWindow(WId wid)
{
    m_backend->restoreWindow(wid);
}

void WindowManager::setKeepAbove(WId wid, bool set)
{
    m_backend->setKeepAbove(wid, set);
}

void WindowManager::moveResizeWindow(WId wid)
{
    m_backend->moveResizeWindow(wid);
}

int WindowManager::numberOfDesktops() const
{
    return m_backend->numberOfDesktops();
}

int WindowManager::currentDesktop() const
{
    return m_backend->currentDesktop();
}

void WindowManager::setCurrentDesktop(int desktop)
{
    m_backend->setCurrentDesktop(desktop);
}

int WindowManager::getDesktopOfWindow(WId wid) const
{
    return m_backend->getDesktopOfWindow(wid);
}

bool WindowManager::isOnCurrentDesktop(WId wid) const
{
    return m_backend->isOnCurrentDesktop(wid);
}

void WindowManager::moveWindowToDesktop(WId wid, int desktop)
{
    m_backend->moveWindowToDesktop(wid, desktop);
}

void WindowManager::createDesktop()
{
    m_backend->createDesktop();
}

void WindowManager::removeDesktop(int deskToRemove)
{
    m_backend->removeDesktop(deskToRemove);
}

QPixmap WindowManager::getWindowPreview(WId wid)
{
    return m_backend->getWindowPreview(wid);
}

QPixmap WindowManager::getWindowIcon(WId wid, const QSize &size)
{
    return m_backend->getWindowIcon(wid, size);
}

void WindowManager::setWindowSkipTaskbar(WId wid, bool set) { m_backend->setWindowSkipTaskbar(wid, set); }
QRect WindowManager::workArea(int desktop) const { return m_backend->workArea(desktop); }
bool WindowManager::isShowingDesktop() const { return m_backend->isShowingDesktop(); }
void WindowManager::setShowingDesktop(bool show) { m_backend->setShowingDesktop(show); }
}  // namespace Common
}  // namespace Kiran
