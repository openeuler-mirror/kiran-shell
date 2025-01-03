# kiran面板
项目包含主面板和面板插件，面板插件有开始菜单、工作区、任务栏、托盘区、日历和显示桌面

## 编译环境要求
  * cmake >= 3.0
  * extra-cmake-modules
  * gcc-c++ >= 4.8
  * kf5-kwindowsystem-devel >= 5.95
  * kf5-kservice-devel
  * kf5-kio-devel
  * kf5-kactivities-devel
  * kf5-kactivities-stats-devel
  * dbusmenu-qt5-devel
  * gsettings-qt-devel
  * kiran-widgets-qt5-devel
  * kiran-log-qt5-devel
  * xcb-util-image-devel
  * libXtst-devel
  * qt5-qtx11extras-devel
  * upower-devel
  * kiran-cc-daemon-devel

## 编译安装
```
# mkdir build
# cd build && cmake -DCMAKE_INSTALL_PREFIX=/usr ..
# make
# make install
# glib-compile-schemas /usr/local/share/glib-2.0/schemas
```

## 运行依赖
  * kactivitymanagerd

## 运行
```
kiran-shell
```

### 默认使用kiran-shell  
将 /usr/share/kiran-session-manager/sessions/kiran.session 中的 kiran-panel 修改为 kiran-shell

## 目录结构

- lib: 维护通用代码
  - waam：waam是windows and applications manager的简称。
  - widgets：通用的自定义控件
- plugins: 面板插件
  - menu：开始菜单
  - taskbar：任务栏
  - systemtray：托盘区域
  - calendar：日期和时间
  - showdesktop：显示桌面
- src：主面板代码
