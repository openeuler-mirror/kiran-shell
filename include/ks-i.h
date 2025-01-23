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
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#pragma once

namespace Kiran
{
#define KS_ICON_MENU "ks-menu"
#define KS_ICON_WORKSPACE_SWITCHER "ks-workspace-switcher"
#define KS_ICON_WORKSPACE_PLUS_SYMBOLIC "ksvg-ks-workspace-plus-symbolic"
#define KS_ICON_MENU_GROUP_SYMBOLIC "ksvg-ks-menu-group-symbolic"

#define KS_ICON_MENU_APPS_LIST_SYMBOLIC "ksvg-ks-menu-apps-list-symbolic"
#define KS_ICON_MENU_RECENT_FILES_SYMBOLIC "ksvg-ks-menu-recent-files-symbolic"
#define KS_ICON_MENU_SETTINGS_SYMBOLIC "ksvg-ks-menu-settings-symbolic"
#define KS_ICON_MENU_LOCK_SYMBOLIC "ksvg-ks-power-lock_screen"
#define KS_ICON_POWER_SWITCH_USER "ksvg-ks-power-switch_user"
#define KS_ICON_POWER_LOGOUT "ksvg-ks-power-logout"
#define KS_ICON_POWER_SUSPEND "ksvg-ks-power-suspend"
#define KS_ICON_POWER_HIBERNATE "ksvg-ks-power-hibernate"
#define KS_ICON_POWER_REBOOT "ksvg-ks-power-reboot"
#define KS_ICON_POWER_SHUTDOWN "ksvg-ks-power-shutdown"
#define KS_ICON_TRAY_BOX "ksvg-ks-tray-box"
#define KS_ICON_TASKLIST_UP_PAGE_SYMBOLIC "ksvg-ks-tasklist-previous-symbolic"
#define KS_ICON_TASKLIST_DOWN_PAGE_SYMBOLIC "ksvg-ks-tasklist-next-symbolic"

#define KS_ICON_WIRED "ksvg-ks-wired"
#define KS_ICON_NET_DISCONNECTED "ksvg-ks-net-disconnected"
#define KS_ICON_WIRED_CONNECTED "ksvg-ks-wired-connected"
#define KS_ICON_WIRED_ERROR "ksvg-ks-wired-error"
#define KS_ICON_WIRELESS "ksvg-ks-wireless"
#define KS_ICON_WIRELESS_PREFIX "ksvg-ks-wireless"
#define KS_ICON_WIRELESS_SECURITY "security"
#define KS_ICON_HWCONF_SETTING "ksvg-ks-hwconf-setting"
#define KS_ICON_HWCONF_SETTING_BACK "ksvg-ks-hwconf-setting-back"
#define KS_ICON_HWCONF_THEME_SWITCH "ksvg-ks-theme-switch"
#define KS_ICON_HWCONF_AUDIO_MUTE "ksvg-ks-audio-mute"
#define KS_ICON_HWCONF_AUDIO_LOW "ksvg-ks-audio-low"
#define KS_ICON_HWCONF_AUDIO_MEDIUM "ksvg-ks-audio-medium"
#define KS_ICON_HWCONF_AUDIO_LOUD "ksvg-ks-audio-loud"
#define KS_ICON_LOADING "ksvg-ks-loading"

#define LOCKDOWN_SCHEMA_ID "com.kylinsec.kiran.shell.lockdown"
#define LOCKDOWN_SCHEMA_KEY_DISABLE_LOCK_SCREEN "disableLockScreen"
#define LOCKDOWN_SCHEMA_KEY_DISABLE_USER_SWITCHING "disableUserSwitching"
#define LOCKDOWN_SCHEMA_KEY_DISABLE_LOG_OUT "disableLogOut"
#define LOCKDOWN_SCHEMA_KEY_DISABLE_SUSPEND "disableSuspend"
#define LOCKDOWN_SCHEMA_KEY_DISABLE_HIBERNATE "disableHibernate"
#define LOCKDOWN_SCHEMA_KEY_DISABLE_REBOOT "disableReboot"
#define LOCKDOWN_SCHEMA_KEY_DISABLE_SHUTDOWN "disableShutdown"

#define SHELL_SCHEMA_ID "com.kylinsec.kiran.shell"
#define SHELL_SCHEMA_KEY_PERSONALITY_MODE "enablePersonalityMode"
#define SHELL_SCHEMA_KEY_AUTO_HIDE "enableAutoHide"
#define SHELL_SCHEMA_KEY_DEFAULT_LAYOUT "defaultLayout"
#define SHELL_SCHEMA_KEY_PANEL_UIDS "panelUids"
#define SHELL_SCHEMA_KEY_APPLET_UIDS "appletUids"

#define POWER_SCHEMA_ID "com.kylinsec.kiran.power"
#define POWER_SCHEMA_TRAY_ICON_POLICY "trayIconPolicy"

#define TASKBAR_SCHEMA_ID "com.kylinsec.kiran.shell.taskbar"
#define TASKBAR_SCHEMA_KEY_SHOW_APP_NAME "showAppName"
#define TASKBAR_SCHEMA_KEY_FIXED_APPS "fixedApps"

#define MENU_SCHEMA_ID "com.kylinsec.kiran.shell.menu"
#define MENU_SCHEMA_KEY_NEW_APPS "newApps"

#define SYSTEMTRAY_SCHEMA_ID "com.kylinsec.kiran.shell.systemtray"
#define SYSTEMTRAY_SCHEMA_KEY_FOLDING_APPS "foldingApps"

#define APPEARANCE_SCHEMA_ID "com.kylinsec.kiran.appearance"
#define APPEARANCE_SCHEMA_KEY_DESKTOP_BACKGROUND "desktopBackground"

#define APPLET_SCHEMA_ID "com.kylinsec.kiran.shell.applet"
#define APPLET_SCHEMA_PATH "/com/kylinsec/kiran/shell/applets"
#define APPLET_SCHEMA_KEY_ID "id"
#define APPLET_SCHEMA_KEY_PANEL "panel"
#define APPLET_SCHEMA_KEY_POSITION "position"
#define APPLET_SCHEMA_KEY_PRS "panelRightStick"

#define PANEL_SCHEMA_ID "com.kylinsec.kiran.shell.panel"
#define PANEL_SCHEMA_PATH "/com/kylinsec/kiran/shell/panels"
#define PANEL_SCHEMA_KEY_SIZE "size"
#define PANEL_SCHEMA_KEY_ORIENTATION "orientation"
#define PANEL_SCHEMA_KEY_MONITOR "monitor"

#define BUTTON_BLANK_SPACE 6

enum PanelOrientation
{
    PANEL_ORIENTATION_TOP,
    PANEL_ORIENTATION_RIGHT,
    PANEL_ORIENTATION_BOTTOM,
    PANEL_ORIENTATION_LEFT
};

}  // namespace Kiran
