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

#include <qt5-log-i.h>
#include <KService/KService>
#include <KWindowInfo>
#include <KWindowSystem/NETWM>
#include <KWindowSystem>
#include <QFileInfo>
#include <QWindow>
#include <QtX11Extras/QX11Info>

#include "utility.h"
#include "window-info-helper.h"

static const NET::Properties windowInfoFlags =
    NET::WMState | NET::XAWMState | NET::WMDesktop | NET::WMVisibleName | NET::WMGeometry | NET::WMFrameExtents | NET::WMWindowType | NET::WMPid;
static const NET::Properties2 windowInfoFlags2 = NET::WM2DesktopFileName | NET::WM2Activities | NET::WM2WindowClass | NET::WM2AllowedActions | NET::WM2AppMenuObjectPath | NET::WM2AppMenuServiceName | NET::WM2GTKApplicationId;

QUrl WindowInfoHelper::getUrlByWId(WId wid)
{
    QByteArray desktopFile = getUrlByWIdPrivate(wid);

    return QUrl::fromLocalFile(desktopFile);
}

QByteArray WindowInfoHelper::getWmClassByWId(WId wid)
{
    QByteArray windowClassName;
    KWindowInfo info(wid, NET::WMPid, NET::WM2WindowClass);
    if (info.valid())
    {
        windowClassName = info.windowClassName();
    }

    if (!windowClassName.isEmpty())
    {
        return windowClassName;
    }

    // /proc/${pid}/status name
    int pid;
    if (info.valid())
    {
        pid = info.pid();
    }
    else
    {
        KLOG_WARNING() << "can't find pid by KWindowInfo:" << wid;
        return "";
    }

    QByteArray procStatus = Utility::runCmd("cat", {QString("/proc/%1/status").arg(pid)});
    procStatus = procStatus.split('\n').first();
    windowClassName = QString::fromLocal8Bit(procStatus).remove("Name:").remove("\t").remove("\n").toLocal8Bit();

    return windowClassName;
}

QString WindowInfoHelper::getAppNameByWId(WId wid)
{
    KWindowInfo info(wid, NET::WMVisibleName);
    if (info.valid())
    {
        return info.visibleName();
    }
    else
    {
        KLOG_WARNING() << "can't find app name by wid:" << wid;
        return "";
    }
}

QString WindowInfoHelper::getAppIconByWId(WId wid)
{
    KWindowInfo info(wid, NET::WMIconName);
    if (info.valid())
    {
        return info.iconName();
    }
    else
    {
        KLOG_WARNING() << "can't find app icon by wid:" << wid;
        return "";
    }
}

bool WindowInfoHelper::hasState(WId wid, NET::States s)
{
    KWindowInfo info(wid, windowInfoFlags, windowInfoFlags2);
    if (info.valid())
    {
        return info.hasState(s);
    }

    return false;
}

bool WindowInfoHelper::isSkipTaskbar(WId wid)
{
    return WindowInfoHelper::hasState(wid, NET::SkipTaskbar);
}

bool WindowInfoHelper::isMinimized(WId wid)
{
    KWindowInfo info(wid, windowInfoFlags, windowInfoFlags2);
    if (info.valid())
    {
        return info.isMinimized();
    }

    return false;
}

bool WindowInfoHelper::isMaximized(WId wid)
{
    return WindowInfoHelper::hasState(wid, NET::Max);
}

bool WindowInfoHelper::isActived(WId wid)
{
    return KWindowSystem::activeWindow() == wid;
}

bool WindowInfoHelper::isKeepAboved(WId wid)
{
    return WindowInfoHelper::hasState(wid, NET::KeepAbove);
}

void WindowInfoHelper::setKeepAbove(WId wid, bool isKeepAbove)
{
    if (isKeepAbove)
    {
        KWindowSystem::setState(wid, NET::KeepAbove);
    }
    else
    {
        KWindowSystem::clearState(wid, NET::KeepAbove);
    }
}

void WindowInfoHelper::closeWindow(WId wid)
{
    switch (KWindowSystem::platform())
    {
    case KWindowSystem::Platform::X11:
    {
        NETRootInfo ri(QX11Info::connection(), NET::CloseWindow);
        ri.closeWindowRequest(wid);
        break;
    }
    case KWindowSystem::Platform::Wayland:
    {
        //TODO:wayland关闭软件
        QWindow *window = QWindow::fromWinId(wid);
        if (window)
        {
            window->close();
        }
        break;
    }
    default:
        break;
    }
}

void WindowInfoHelper::maximizeWindow(WId wid, bool isMaximized)
{
    if (isMaximized)
    {
        KWindowSystem::setState(wid, NET::Max);
    }
    else
    {
        KWindowSystem::clearState(wid, NET::Max);
    }

    KWindowSystem::activateWindow(wid);
}

void WindowInfoHelper::minimizeWindow(WId wid)
{
    KWindowSystem::minimizeWindow(wid);
}

void WindowInfoHelper::activateWindow(WId wid)
{
    KWindowSystem::activateWindow(wid);
}

WId WindowInfoHelper::activeWindow()
{
    return KWindowSystem::activeWindow();
}

QByteArray WindowInfoHelper::getUrlByWIdPrivate(WId wid)
{
    QByteArray desktopFile;

    KWindowInfo info(wid, NET::WMPid, NET::WM2DesktopFileName | NET::WM2WindowClass);
    desktopFile = info.desktopFileName();
    if (!desktopFile.isEmpty())
    {
        return desktopFile;
    }

    int pid;
    if (info.valid())
    {
        pid = info.pid();
    }
    else
    {
        KLOG_WARNING() << "can't find pid by Wid";
        return "";
    }

    desktopFile = getDesktopFileByEnviorn(pid);
    if (!desktopFile.isEmpty())
    {
        return desktopFile;
    }

    desktopFile = getDesktopFileByCmdline(pid);
    if (!desktopFile.isEmpty())
    {
        return desktopFile;
    }

    KLOG_WARNING() << "can't find url by Wid:" << wid;

    return desktopFile;
}

QByteArray WindowInfoHelper::getDesktopFileByEnviorn(int pid)
{
    // /proc/${pid}/enviorn GIO_LAUNCHED_DESKTOP_FILE
    QByteArray desktopFileEnv = "GIO_LAUNCHED_DESKTOP_FILE=";

    QByteArray environ = Utility::runCmd("cat", {QString("/proc/%1/environ").arg(pid)});
    QByteArrayList envsList = environ.split('\0');
    for (auto env : envsList)
    {
        if (env.startsWith(desktopFileEnv))
        {
            return env.mid(desktopFileEnv.length());
        }
    }

    return "";
}

QByteArray WindowInfoHelper::getDesktopFileByCmdline(int pid)
{
    QByteArray cmdline = Utility::runCmd("cat", {QString("/proc/%1/cmdline").arg(pid)});
    QByteArrayList envsList = cmdline.split(' ');
    QString cmd = envsList.first();
    if (cmd.isEmpty())
    {
        return "";
    }
    QString cmd_simple = QFileInfo(cmd).fileName();

    // 按优先级一遍一遍过滤，不能在一个循环中做所有的过滤，不然优先级低的可能会命中
    // 1.service->desktopEntryName()
    // 2.service->name()
    // 3.service->exec()
    auto allKService = KService::allServices();
    for (auto service : allKService)
    {
        //        KLOG_INFO() << service->storageId() << service->desktopEntryName() << service->entryPath() << service->keywords() << service->name();
        QString desktopEntryName = service->desktopEntryName();
        if (desktopEntryName.isEmpty())
        {
            continue;
        }
        if (cmd_simple == desktopEntryName)
        {
            return service->entryPath().toLocal8Bit();
        }
    }

    for (auto service : allKService)
    {
        QString serviceName = service->exec();
        if (serviceName.isEmpty())
        {
            continue;
        }
        if (cmd_simple == serviceName)
        {
            return service->entryPath().toLocal8Bit();
        }
    }

    for (auto service : allKService)
    {
        QString exec = service->exec();
        if (exec.isEmpty())
        {
            continue;
        }
        if (cmd_simple == exec)
        {
            return service->entryPath().toLocal8Bit();
        }
    }

    for (auto service : allKService)
    {
        QString exec = service->exec();
        if (exec.isEmpty())
        {
            continue;
        }
        QString exec_simple = exec.mid(0, exec.indexOf(" "));

        if (cmd_simple == exec_simple || cmd_simple.startsWith(exec_simple) || exec_simple.startsWith(cmd_simple))
        {
            return service->entryPath().toLocal8Bit();
        }
    }

    return "";
}
