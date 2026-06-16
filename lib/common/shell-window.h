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

#pragma once

#include <QEvent>
#include <QWidget>
#include <QPoint>

class QMoveEvent;
class QPlatformSurfaceEvent;

namespace KWayland {
namespace Client {
class PlasmaShellSurface;
class ConnectionThread;
class Registry;
class PlasmaShell;
}
}

namespace Kiran
{

enum class ShellWindowRole {
    Panel,
    AppletPopup,
};

class ShellWindow : public QWidget {
    Q_OBJECT
    Q_DISABLE_COPY(ShellWindow)
public:
    explicit ShellWindow(ShellWindowRole role, QWidget *parent = nullptr);
    ~ShellWindow() override;

    ShellWindowRole role() const { return m_role; }

    void setPosition(const QPoint &pos);
    void setPosition(int x, int y);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    bool event(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void createShellSurface();
    void destroyShellSurface();
    void ensureWaylandConnection();
    void syncShellSurface();
    void handlePlatformSurfaceEvent(QPlatformSurfaceEvent *event);
    void handleMoveEvent(QMoveEvent *event);

    Qt::WindowFlags suggestedFlags(ShellWindowRole role) noexcept;

    ShellWindowRole m_role;
    bool m_wayland;
    KWayland::Client::PlasmaShellSurface *m_shellSurface = nullptr;
    QPoint m_pendingPosition;

    // Wayland 连接对象，所有 ShellWindow 实例共享
    static KWayland::Client::ConnectionThread *s_connection;
    static KWayland::Client::Registry *s_registry;
    static KWayland::Client::PlasmaShell *s_plasmaShell;
    static bool s_waylandInitialized;
};

}  // namespace Kiran
