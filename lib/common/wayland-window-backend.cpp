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

#include <QGuiApplication>
#include <QWindow>

#include "wayland-window-backend.h"

namespace Kiran
{
namespace Common
{
WaylandWindowBackend::WaylandWindowBackend(QObject* parent)
    : WindowManagerBackend(parent)
{
}

WaylandWindowBackend::~WaylandWindowBackend() = default;

QList<WId> WaylandWindowBackend::getAllWindows() const
{
    qWarning("WaylandWindowBackend::getAllWindows not implemented");
    return {};
}

QList<WId> WaylandWindowBackend::getAllWindows(int desktop) const
{
    Q_UNUSED(desktop);
    qWarning("WaylandWindowBackend::getAllWindows not implemented");
    return {};
}

QRect WaylandWindowBackend::getWindowGeometry(WId wid) const
{
    Q_UNUSED(wid);
    qWarning("WaylandWindowBackend::getWindowGeometry not implemented");
    return {};
}

QString WaylandWindowBackend::getWindowAppId(WId wid) const
{
    Q_UNUSED(wid);
    qWarning("WaylandWindowBackend::getWindowAppId not implemented");
    return {};
}

QString WaylandWindowBackend::getWindowTitle(WId wid) const
{
    Q_UNUSED(wid);
    qWarning("WaylandWindowBackend::getWindowTitle not implemented");
    return {};
}

QString WaylandWindowBackend::getWindowIconName(WId wid) const
{
    Q_UNUSED(wid);
    qWarning("WaylandWindowBackend::getWindowIconName not implemented");
    return {};
}

QByteArray WaylandWindowBackend::getWindowDesktopFileName(WId wid) const
{
    Q_UNUSED(wid);
    qWarning("WaylandWindowBackend::getWindowDesktopFileName not implemented");
    return {};
}

int WaylandWindowBackend::getWindowPid(WId wid) const
{
    Q_UNUSED(wid);
    qWarning("WaylandWindowBackend::getWindowPid not implemented");
    return -1;
}

bool WaylandWindowBackend::isSkipTaskbar(WId wid) const
{
    Q_UNUSED(wid);
    qWarning("WaylandWindowBackend::isSkipTaskbar not implemented");
    return false;
}

bool WaylandWindowBackend::isMinimized(WId wid) const
{
    Q_UNUSED(wid);
    qWarning("WaylandWindowBackend::isMinimized not implemented");
    return false;
}

bool WaylandWindowBackend::isMaximized(WId wid) const
{
    Q_UNUSED(wid);
    qWarning("WaylandWindowBackend::isMaximized not implemented");
    return false;
}

bool WaylandWindowBackend::isKeepAbove(WId wid) const
{
    Q_UNUSED(wid);
    qWarning("WaylandWindowBackend::isKeepAbove not implemented");
    return false;
}

bool WaylandWindowBackend::isActive(WId wid) const
{
    Q_UNUSED(wid);
    qWarning("WaylandWindowBackend::isActive not implemented");
    return false;
}

WId WaylandWindowBackend::activeWindow() const
{
    qWarning("WaylandWindowBackend::activeWindow not implemented");
    return 0;
}

void WaylandWindowBackend::closeWindow(WId wid)
{
    auto* window = QWindow::fromWinId(wid);
    if (window)
    {
        window->close();
    }
}

void WaylandWindowBackend::activateWindow(WId wid)
{
    Q_UNUSED(wid);
    qWarning("WaylandWindowBackend::activateWindow not implemented");
}

void WaylandWindowBackend::minimizeWindow(WId wid)
{
    Q_UNUSED(wid);
    qWarning("WaylandWindowBackend::minimizeWindow not implemented");
}

void WaylandWindowBackend::maximizeWindow(WId wid, bool set)
{
    Q_UNUSED(wid);
    Q_UNUSED(set);
    qWarning("WaylandWindowBackend::maximizeWindow not implemented");
}

void WaylandWindowBackend::restoreWindow(WId wid)
{
    Q_UNUSED(wid);
    qWarning("WaylandWindowBackend::restoreWindow not implemented");
}

void WaylandWindowBackend::setKeepAbove(WId wid, bool set)
{
    Q_UNUSED(wid);
    Q_UNUSED(set);
    qWarning("WaylandWindowBackend::setKeepAbove not implemented");
}

void WaylandWindowBackend::moveResizeWindow(WId wid)
{
    Q_UNUSED(wid);
    qWarning("WaylandWindowBackend::moveResizeWindow not implemented");
}

int WaylandWindowBackend::numberOfDesktops() const
{
    qWarning("WaylandWindowBackend::numberOfDesktops not implemented");
    return 0;
}

int WaylandWindowBackend::currentDesktop() const
{
    qWarning("WaylandWindowBackend::currentDesktop not implemented");
    return 0;
}

void WaylandWindowBackend::setCurrentDesktop(int desktop)
{
    Q_UNUSED(desktop);
    qWarning("WaylandWindowBackend::setCurrentDesktop not implemented");
}

int WaylandWindowBackend::getDesktopOfWindow(WId wid) const
{
    Q_UNUSED(wid);
    qWarning("WaylandWindowBackend::getDesktopOfWindow not implemented");
    return 0;
}

bool WaylandWindowBackend::isOnCurrentDesktop(WId wid) const
{
    Q_UNUSED(wid);
    qWarning("WaylandWindowBackend::isOnCurrentDesktop not implemented");
    return false;
}

void WaylandWindowBackend::moveWindowToDesktop(WId wid, int desktop)
{
    Q_UNUSED(wid);
    Q_UNUSED(desktop);
    qWarning("WaylandWindowBackend::moveWindowToDesktop not implemented");
}

void WaylandWindowBackend::createDesktop()
{
    qWarning("WaylandWindowBackend::createDesktop not implemented");
}

void WaylandWindowBackend::removeDesktop(int deskToRemove)
{
    Q_UNUSED(deskToRemove);
    qWarning("WaylandWindowBackend::removeDesktop not implemented");
}

void WaylandWindowBackend::setWindowSkipTaskbar(WId wid, bool set)
{
    Q_UNUSED(wid);
    Q_UNUSED(set);
    qWarning("WaylandWindowBackend::setWindowSkipTaskbar not implemented");
}

QRect WaylandWindowBackend::workArea(int desktop) const
{
    Q_UNUSED(desktop);
    qWarning("WaylandWindowBackend::workArea not implemented");
    return {};
}

bool WaylandWindowBackend::isShowingDesktop() const
{
    qWarning("WaylandWindowBackend::isShowingDesktop not implemented");
    return false;
}

void WaylandWindowBackend::setShowingDesktop(bool show)
{
    Q_UNUSED(show);
    qWarning("WaylandWindowBackend::setShowingDesktop not implemented");
}

QPixmap WaylandWindowBackend::getWindowIcon(WId wid, const QSize &size)
{
    Q_UNUSED(wid);
    Q_UNUSED(size);
    qWarning("WaylandWindowBackend::getWindowIcon not implemented");
    return {};
}

QPixmap WaylandWindowBackend::getWindowPreview(WId wid)
{
    Q_UNUSED(wid);
    qWarning("WaylandWindowBackend::getWindowPreview not implemented");
    return {};
}
}  // namespace Common
}  // namespace Kiran
