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
#include <QByteArray>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QString>
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
    static void moveResize(WId wid);

    static void activateWindow(WId wid);
    static WId activeWindow();

    static int getDesktopOfWindow(WId wid);
    static bool isOnCurrentDesktop(WId wid);

private:
    WindowInfoHelper() {}

    static QByteArray getUrlByWIdPrivate(WId wid);

    static QByteArray getDesktopFileByInfoStr(QString info);
    static QByteArray getDesktopFileByEnviorn(int pid);
    static QByteArray getDesktopFileByCmdline(int pid);
    static QByteArray getDesktopFileByWmClass(QStringList classNames);

private:
    // KService 缓存数据结构，用于快速查询
    static void initServiceCache();
    static void reloadServiceCache();          // 重新加载缓存（内部会获取锁）
    static void reloadServiceCacheUnlocked();  // 重新加载缓存（不获取锁，需要在锁保护下调用）
    static QByteArray findDesktopFileByInfo(const QString& info);
    static QByteArray queryFromCache(const QString& info);  // 从缓存中查询（需要在锁保护下调用）

    // 静态缓存映射表
    static QMap<QString, QByteArray> s_desktopEntryNameMap;  // 第一级：desktopEntryName -> entryPath
    static QMap<QString, QByteArray> s_serviceNameMap;       // 第二级：name -> entryPath
    static QMap<QString, QByteArray> s_execMap;              // 第三级：exec -> entryPath
    static QMap<QString, QByteArray> s_execSimpleMap;        // 第三级：exec_simple -> entryPath
    static QMap<QString, QByteArray> s_startupWMClassMap;    // 第四级：StartupWMClass -> entryPath
    static bool s_serviceCacheInitialized;                   // 缓存是否已初始化
    static QMutex s_cacheMutex;                              // 缓存互斥锁，用于数据安全
                                                             // 虽是单线程，但当查询是由信号触发时，可能存在嵌套调用，需要锁保护
};
