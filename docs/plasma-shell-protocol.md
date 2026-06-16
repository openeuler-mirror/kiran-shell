# Plasma Shell 协议支持说明

## 概述

Plasma Shell 协议是 KDE 定义的 Wayland 扩展协议，用于 Shell 面板与 Compositor 之间的通信。本项目中通过 `ShellWindow` 基类实现对协议的支持，使面板窗口在 Wayland 下能够正确定位和管理。

协议 XML 定义见 `docs/protocols/plasma-shell.xml`。

## 协议接口

### org_kde_plasma_shell（全局接口）

全局单例，只能绑定一次。用于创建 shell surface。

| 请求 | 说明 |
|---|---|
| `get_surface(surface)` → `org_kde_plasma_surface` | 为指定的 wl_surface 创建 shell surface |

### org_kde_plasma_surface（每个 shell 窗口一个）

| 请求 | 说明 |
|---|---|
| `destroy()` | 销毁 shell surface（必须在 wl_surface 销毁前调用） |
| `set_output(output)` | 关联到指定输出（显示器） |
| `set_position(x, y)` | 设置全局坐标 |
| `set_role(role)` | 设置窗口角色（只能设置一次） |
| `set_panel_behavior(behavior)` | 设置面板行为 |
| `set_skip_taskbar(skip)` | 设置是否跳过任务栏 |
| `set_panel_takes_focus(takes_focus)` | 设置面板是否可获取焦点 |
| `open_under_cursor()` | 请求在光标位置打开 |

### 角色枚举

| 值 | 名称 | 说明 |
|---|---|---|
| 0 | normal | 普通窗口 |
| 1 | desktop | 桌面 |
| 2 | panel | 面板 |
| 3 | onscreendisplay | OSD |
| 4 | notification | 通知 |
| 5 | tooltip | 工具提示 |
| 6 | criticalnotification | 关键通知 |
| 7 | appletpopup | 插件弹窗 |

### 面板行为枚举

| 值 | 名称 | 说明 |
|---|---|---|
| 1 | always_visible | 面板始终可见，窗口不能覆盖 |
| 2 | auto_hide | 面板自动隐藏 |
| 3 | windows_can_cover | 窗口可覆盖面板 |
| 4 | windows_go_below | 最大化窗口不覆盖面板 |

## ShellWindow 适配实现

### 类结构

`ShellWindow` 继承 `QWidget`，内部通过 `m_wayland` 标志统一判断平台路径。

```
ShellWindow : QWidget
├── m_wayland: bool              ← 构造函数中通过 KWindowSystem::isPlatformWayland() 计算
├── m_shellSurface               ← KWayland::Client::PlasmaShellSurface*
├── m_pendingPosition            ← 待应用的窗口位置
├── setPosition(x, y)            ← 统一入口，X11/Wayland 双路径
└── createShellSurface()         ← Wayland 下创建 PlasmaShellSurface
```

### Wayland 连接管理

所有 `ShellWindow` 实例共享同一个 Wayland 连接，通过静态单例管理：

```
ConnectionThread (fromApplication)
    └── Registry
            └── plasmaShellAnnounced → createPlasmaShell → PlasmaShell
```

`ensureWaylandConnection()` 在首次创建 shell surface 时自动初始化。

### Surface 生命周期

```
QEvent::PlatformSurface
├── SurfaceCreated
│   └── createShellSurface()
│       ├── ensureWaylandConnection()
│       ├── Surface::fromWindow(windowHandle())  → 获取 wl_surface
│       ├── s_plasmaShell->createSurface()       → 创建 PlasmaShellSurface
│       ├── setRole(Panel / AppletPopup)
│       ├── setPanelBehavior(AlwaysVisible)       ← Panel 角色
│       ├── setSkipTaskbar(true)
│       └── setPosition(m_pendingPosition)
│
└── SurfaceAboutToBeDestroyed
    └── destroyShellSurface()
        └── m_shellSurface->deleteLater()
```

### 定位流程

`setPosition(x, y)` 是 X11 和 Wayland 的统一入口：

```
ShellWindow::setPosition(x, y)
├── m_pendingPosition = (x, y)
├── QWidget::move(x, y)                    ← X11: Qt 直接移动窗口
└── if (m_shellSurface)
    └── m_shellSurface->setPosition(x, y)  ← Wayland: 通知 Compositor 移动 surface
```

### ShellWindowRole 映射

| ShellWindowRole | PlasmaShellSurface::Role | X11 等价行为 |
|---|---|---|
| Panel | Panel | `KWindowSystem::setType(NET::Dock)` + `setOnAllDesktops` + `setExtendedStrut` |
| AppletPopup | AppletPopup | `Qt::Popup` / `Qt::ToolTip` 窗口 flags |

## X11 回退

X11 下通过以下方式实现等价行为：

| Wayland 协议请求 | X11 等价实现 | 位置 |
|---|---|---|
| `set_role(panel)` | `KWindowSystem::setType(winId(), NET::Dock)` | ShellWindow 构造函数 |
| — | `KWindowSystem::setOnAllDesktops(winId(), true)` | ShellWindow 构造函数 |
| `set_position(x, y)` | `QWidget::move(x, y)` | ShellWindow::setPosition |
| `set_panel_behavior(always_visible)` | `KWindowSystem::setExtendedStrut(...)` | Panel::updateGeometry |

## 关键约束

- `get_surface` 必须在 `wl_surface` 创建后调用。通过监听 `QEvent::PlatformSurface(SurfaceCreated)` 保证时序。
- `destroy()` 必须在 `wl_surface` 销毁前调用。通过监听 `QEvent::PlatformSurface(SurfaceAboutToBeDestroyed)` 保证时序。
- `set_role` 只能调用一次，重复调用会失败。
- 所有 `ShellWindow` 实例共享同一个 Wayland 连接（`ConnectionThread`、`Registry`、`PlasmaShell` 均为静态单例）。
- `ShellWindow` 与 `WindowManager` 正交：前者管理面板自身窗口，后者管理第三方窗口。

## 依赖

- `KF5::WaylandClient` — KWayland Client 库，提供 `PlasmaShell`、`PlasmaShellSurface`、`Surface` 等类
- `wayland-client` — Wayland 协议客户端库
- `KWindowSystem` — X11 回退路径依赖
