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

#pragma once

#include <QDir>

#define BUTTON_BLANK_SPACE 6

// 配置文件 及其字段
#define KIRAN_SHELL_SETTING_FILE QDir::homePath() + "/.config/kiran-shell/kiran-shell.ini"
#define MENU_NEW_APP "menu/newApp"
#define TASKBAR_SHOW_APP_BTN_TAIL_KEY "taskbar/showAppBtnTail"
#define TASKBAR_LOCK_APP_KEY "taskbar/lockApp"
#define SYSTEM_TRAY_WINDOW_POPUP_ITEMS_KEY "systemtray/trayBoxItems"
