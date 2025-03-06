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

#include <KWindowInfo>
#include <QObject>
#include <QWidgetList>  //For WId

class WindowInfoHelper : public QObject
{
    Q_OBJECT
public:
    static QUrl getUrlByWId(WId wid);
    static QByteArray getWmClassByWId(WId wid);
    static QString getAppNameByWId(WId wid);
    static QString getAppIconByWId(WId wid);

    static bool hasState(WId wid, NET::States s);
    static bool isSkipTaskbar(WId wid);
    static bool isMinimized(WId wid);
    static bool isMaximized(WId wid);
    static bool isKeepAboved(WId wid);
    static bool isActived(WId wid);

    static void setKeepAbove(WId wid, bool isKeepAbove);
    static void closeWindow(WId wid);
    static void maximizeWindow(WId wid, bool isMaximized);
    static void minimizeWindow(WId wid);
    static void restoredWindow(WId wid);

    static void activateWindow(WId wid);
    static WId activeWindow();

    static int getDesktopOfWindow(WId wid);

private:
    WindowInfoHelper() {}

    static QByteArray getUrlByWIdPrivate(WId wid);

    static QByteArray getDesktopFileByEnviorn(int pid);
    static QByteArray getDesktopFileByCmdline(int pid);
};
