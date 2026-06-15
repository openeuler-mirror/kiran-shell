# kiran-shell 架构说明

## 项目定位

`kiran-shell` 是 Kiran 桌面环境中的 Shell / 面板组件，负责提供桌面面板主进程、面板插件宿主能力、常用面板插件以及与桌面环境相关的集成能力。

项目运行在 Linux 桌面环境中，依赖 Qt5、KF5、DBus、GSettings、X11/XCB 等基础能力，并需要兼顾窗口管理、系统托盘、应用启动、桌面会话和多屏 / HiDPI 等运行场景。

## 总体结构

项目主要由以下部分组成：

- `src/shell`：主面板进程 `kiran-shell`，负责面板窗口、布局加载、插件加载和 applet 生命周期管理。
- `src/shelld`：后台服务 `kiran-shelld`，包含 StatusNotifierWatcher 等系统托盘相关服务能力。
- `plugins`：面板插件集合，包括开始菜单、任务栏、工作区、系统托盘、日历、显示桌面、设置栏和占位插件。
- `lib/common`：通用工具库，封装应用启动、窗口信息、窗口管理、图标处理、DBus service watcher、通知和日志分类等能力。
- `lib/widgets`：面向插件和面板复用的 Qt 控件。
- `data`：默认布局、desktop 文件、GSettings schema、菜单数据、托盘翻译配置等运行数据。
- `dbus`：DBus XML 接口定义和部分 DBus 类型声明。
- `resources`：Qt 资源文件。
- `third_party`：第三方代码，目前包含 XEmbed / StatusNotifierItem 兼容相关组件。

## 主面板进程

`src/shell` 是面板主进程的核心目录。

主面板进程的职责包括：

- 初始化 `kiran-shell` 应用进程。
- 创建和管理面板窗口。
- 读取布局配置并创建 panel / applet 实例。
- 根据 applet id 查找并加载对应插件。
- 将插件挂载到面板布局中。
- 维护面板方向、位置、尺寸、右侧吸附等布局行为。

布局相关代码集中在 `src/shell/profile`。默认布局由 `data/default.layout` 提供，运行时根据配置中的 panel、applet、position、panel-right-stick 等字段决定面板和插件的组织方式。

插件加载由 `PluginPool` 管理。它扫描安装目录中的插件共享库，读取 Qt 插件元数据中的 `plugin_id` 和 `applets` 信息，再根据 applet id 延迟加载对应插件实例。

## 插件体系

插件位于 `plugins/<name>`，通常构建为共享库，并通过 Qt 插件元数据描述自身能力。

典型插件包含：

- 插件源码和 UI 文件。
- 插件目录下的 `CMakeLists.txt`。
- 插件元数据 JSON，例如 `plugin_id` 和 `applet_id`。
- 可选的 GSettings schema、DBus 接口、翻译文件等配套资源。

当前主要插件职责如下：

- `plugins/menu`：开始菜单，负责应用菜单、搜索和菜单相关 DBus 能力。
- `plugins/taskbar`：任务栏，负责固定应用、运行中窗口、窗口激活 / 最小化、预览和分组显示。
- `plugins/workspace`：工作区，负责虚拟桌面 / 工作区展示与切换。
- `plugins/systemtray`：系统托盘，负责托盘项展示，并与 StatusNotifierItem / XEmbed 兼容逻辑配合。
- `plugins/calendar`：日历 / 时钟展示。
- `plugins/settingbar`：设置栏，负责网络、声音、电源等快捷设置入口。
- `plugins/showdesktop`：显示桌面。
- `plugins/spacer`：面板占位和布局辅助。

插件 ID、applet ID、默认布局和加载逻辑必须保持一致。新增或修改插件时，应同时检查插件 JSON、`data/default.layout`、相关 schema、CMake 安装逻辑和运行时加载路径。

## 后台服务与系统托盘

`src/shelld` 构建 `kiran-shelld`，用于承载不适合放在主面板进程中的后台服务逻辑。

当前重点能力是 StatusNotifierWatcher。它负责在 DBus 上提供 StatusNotifierWatcher 服务，使支持 StatusNotifierItem 协议的应用能够注册托盘项，并由系统托盘插件进行展示。

系统托盘相关逻辑需要同时考虑：

- StatusNotifierItem 协议。
- XEmbed fallback。
- watcher 服务启动时机。
- 托盘应用注册、注销和服务异常退出。
- 与 `third_party/xembed-sni-proxy` 的兼容边界。

## 通用库

`lib/common` 提供跨主进程和插件复用的基础能力。

常见职责包括：

- 应用启动和 desktop 文件处理。
- 图标查找和主题图标 fallback。
- **窗口管理**：通过 `WindowManager` 门面单例统一封装全部第三方窗口操作，内部通过 `WindowManagerBackend` 接口解耦 X11 / Wayland 平台差异。
- **桌面文件缓存**：`DesktopFileCache` 提供独立的 KService 缓存查询，不依赖窗口系统 API。
- DBus service watcher。
- 通知封装。
- 日志分类。

`lib/widgets` 提供通用 UI 控件，例如省略文本标签、加载标签、悬停滑块、窗口缩略图和样式化按钮。

新增功能时应优先复用这些公共能力，避免在插件中重复实现窗口、图标、应用启动或 DBus watcher 逻辑。

## 窗口管理层

### 架构设计

窗口管理层采用 Facade 设计模式，通过三层结构解耦平台依赖：

```
WindowManager (门面单例)         ← 公开 API，所有调用方通过此入口
    └── WindowManagerBackend (纯虚接口)  ← 平台无关接口定义
            ├── X11WindowBackend        ← X11 后端实现
            └── WaylandWindowBackend    ← Wayland 后端实现 (stub)
```

**设计原则：**
- 所有平台相关类型（`KWindowSystem`、`NET::Properties`、`KWindowInfo`、`KX11Extras`、`QX11Info`）仅允许出现在后端实现文件中，不得泄漏到业务层。
- 信号使用平台无关参数（`WId` 而非 `NET::Properties`），通过具体信号（`windowTitleChanged`、`windowStateChanged`、`windowDesktopChanged` 等）传递变更信息。
- 插件和业务层通过 `WindowManagerInstance` 宏访问门面，不直接调用 KWindowSystem。

### WindowManagerBackend 接口

纯虚接口定义所有窗口管理操作，按功能分组：

| 分组 | 方法 |
|---|---|
| 窗口跟踪 | `getAllWindows`、`getWindowGeometry` |
| 窗口标识 | `getWindowAppId`、`getWindowTitle`、`getWindowIconName`、`getWindowDesktopFileName`、`getWindowPid` |
| 窗口状态 | `isSkipTaskbar`、`isMinimized`、`isMaximized`、`isKeepAbove`、`isActive`、`activeWindow` |
| 窗口操作 | `closeWindow`、`activateWindow`、`minimizeWindow`、`maximizeWindow`、`restoreWindow`、`setKeepAbove`、`moveResizeWindow` |
| 工作区管理 | `numberOfDesktops`、`currentDesktop`、`setCurrentDesktop`、`getDesktopOfWindow`、`isOnCurrentDesktop`、`moveWindowToDesktop`、`createDesktop`、`removeDesktop` |
| 窗口属性 | `setWindowSkipTaskbar`、`workArea`、`isShowingDesktop`、`setShowingDesktop` |
| 窗口图标 | `getWindowIcon`（读取 `_NET_WM_ICON`） |
| 窗口截图 | `getWindowPreview`（每次调用执行 xcb_image_get） |

信号列表：`windowAdded`、`windowRemoved`、`activeWindowChanged`、`windowTitleChanged`、`windowIconChanged`、`windowStateChanged`、`windowGeometryChanged`、`windowDesktopChanged`、`windowChanged`（兜底）、`currentDesktopChanged`、`numberOfDesktopsChanged`。

### X11 后端

`X11WindowBackend` 封装全部 X11 平台操作：

- **窗口跟踪**：通过 `KWindowSystem` 信号监听窗口添加/移除/变更。
- **窗口集合**：使用 `QSet<WId> m_managedWindows` 管理受管窗口，替代原 `QMap<WId, QPixmap>` 的双重职责。
- **截图**：`getWindowPreview` 每次调用执行 `xcb_image_get`，不做缓存。
- **图标**：`getWindowIcon` 调用 `KWindowSystem::icon()` 读取 `_NET_WM_ICON` 属性。
- **信号转换**：`changedWindow` 将 `NET::Properties` 掩码转换为具体信号发射。

### Wayland 后端

`WaylandWindowBackend` 为初始 stub 实现，所有方法返回空值 + `qWarning`。`closeWindow` 保留 `QWindow::fromWinId(wid)->close()` 实现。

### DesktopFileCache

从 `WindowInfoHelper` 中提取的纯数据查询工具，不依赖任何窗口系统 API：

- `findByAppId(appId)`：四级缓存查询（desktopEntryName → serviceName → exec → startupWMClass）。
- `findByPid(pid)`：读取 `/proc/pid/environ` 和 `/proc/pid/cmdline` 后查缓存。
- `findByDesktopEntryName(entryName)`：直接查 desktopEntryName 映射。
- `findByExec(exec)`：通过 exec 路径查缓存。

### 图标获取流程

`getWindowAppIcon(wid, size)` 的完整回退链：

1. 通过 `getAppInfo` 获取 desktop 文件路径：
   - `getWindowDesktopFileName(wid)` — 窗口属性 `_KDE_NET_WM_DESKTOP_FILE` 直接提供。
   - `DesktopFileCache::findByAppId(appId)` — 通过 WM_CLASS 查 KService 缓存。
   - `DesktopFileCache::findByPid(pid)` — 通过 PID 查 cmdline/environ。
2. 从 desktop 文件的 `Icon` 字段加载主题图标。
3. `getWindowIconName(wid)` → `QIcon::fromTheme()` — 尝试 WM_ICON_NAME 作为主题图标名。
4. `getWindowIcon(wid, size)` → `KWindowSystem::icon()` — 读取 `_NET_WM_ICON` 属性。

### 迁移说明

原 `WindowInfoHelper` 和 `DesktopHelper` 已删除，其功能分别迁移到：

- 窗口信息查询 → `WindowManager`
- 窗口操作 → `WindowManager`
- 工作区管理 → `WindowManager`
- KService 缓存 → `DesktopFileCache`

详细迁移映射见 `docs/superpowers/specs/2026-06-12-kiran-shell-window-management-refactoring-complete.md`。

## 配置与数据

项目运行数据主要位于 `data`。

关键文件包括：

- `data/default.layout`：默认面板和 applet 布局。
- `data/schemas`：GSettings schema，定义 shell、panel、applet、menu、taskbar、systemtray、settingbar 等配置。
- `data/kiran-shell.desktop.in`：桌面启动文件模板。
- `data/user.conf`：用户配置。
- `data/applications-menu`：菜单相关数据。
- `data/tray-translation.ini`：托盘名称翻译配置。

修改 GSettings schema 时，需要同步检查 key 类型、默认值、description 和所有调用方。修改默认布局时，需要确认插件 id、applet id、位置、右侧吸附和面板方向等字段与插件实现一致。

## DBus 集成

DBus XML 接口定义集中在 `dbus`，部分插件或服务目录下也可能包含自身接口文件。

项目涉及的 DBus 场景包括：

- StatusNotifierWatcher / StatusNotifierItem。
- SessionDaemon 和 SystemDaemon 的音频、电源、时间、账户、外观等接口。
- UPower、login1、DisplayManager、GNOME SessionManager 等桌面系统接口。
- 菜单插件对外提供的接口。

修改 DBus 接口时，需要同步检查服务端、客户端、生成代码、CMake 规则和所有调用方。

## 窗口系统与运行环境

项目需要与桌面窗口系统深度集成。

窗口管理操作统一通过 `WindowManager` 门面单例访问，内部通过 `WindowManagerBackend` 接口解耦 X11 / Wayland 平台差异。所有平台相关类型仅出现在后端实现中，业务层不直接调用 KWindowSystem。

重点运行场景包括：

- X11 / XCB 窗口信息查询与窗口操作（封装在 `X11WindowBackend`）。
- Wayland 相关协议文档和兼容场景（`WaylandWindowBackend` 为 stub，后续逐步实现）。
- 多屏环境下面板位置和几何计算。
- HiDPI / 缩放环境下尺寸、坐标和图标显示。
- 任务栏窗口激活、最小化、预览、分组和桌面切换行为。
- 面板横向 / 纵向布局切换。

涉及窗口、面板几何、任务栏或托盘行为的修改，应优先确认这些场景是否受影响，并在无法实际验证时明确说明未验证项。

## 构建关系

顶层 `CMakeLists.txt` 负责查找 Qt5、KF5、DBus、XCB、GSettings、Kiran 相关依赖，并按顺序加入以下子目录：

- `data`
- `resources`
- `lib`
- `src`
- `plugins`
- `third_party`

主要构建产物包括：

- `kiran-shell`：主面板进程。
- `kiran-shelld`：后台服务进程。
- `lib/common`：共享通用库。
- `lib/widgets`：静态控件库。
- 各插件共享库。
- `xembedsniproxy`：第三方托盘兼容组件。

## 修改影响范围建议

修改前可按以下方向判断影响范围：

- 改 `src/shell`：重点关注面板生命周期、布局加载、插件加载、多屏和 HiDPI。
- 改 `src/shelld`：重点关注 DBus 服务名、StatusNotifierWatcher 生命周期和托盘注册流程。
- 改 `plugins/taskbar`：重点关注窗口激活、最小化、预览、固定应用、虚拟桌面和分组逻辑。
- 改 `plugins/systemtray`：重点关注 SNI、XEmbed fallback、DBus watcher 和托盘项生命周期。
- 改 `plugins/menu`：重点关注应用索引、图标 fallback、DBus 菜单接口和启动行为。
- 改 `data/default.layout`：重点关注插件顺序、右侧吸附和默认 applet id。
- 改 `data/schemas`：重点关注默认值、运行时读取路径和兼容性。
- 改 `dbus`：重点关注接口生成代码、服务端和客户端同步。
- 改 `third_party`：需确认问题确实来自第三方组件，避免无关改动。
- **改 `lib/common` 窗口管理层**：重点关注 `WindowManagerBackend` 接口变更对 X11/Wayland 后端的影响、信号签名变更对插件连接的影响、以及 `DesktopFileCache` 查询逻辑变更对图标获取的影响。修改后端实现时需确保平台类型不泄漏到业务层。
