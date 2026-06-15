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
#include <QByteArray>
#include <QObject>
#include <QPixmap>
#include <QRect>
#include <QSize>
#include <QString>
#include <QWidgetList>

class WindowManagerBackend : public QObject
{
    Q_OBJECT
public:
    explicit WindowManagerBackend(QObject *parent = nullptr);
    ~WindowManagerBackend() override;

    virtual QList<WId> getAllWindows() const = 0;
    virtual QList<WId> getAllWindows(int desktop) const = 0;
    virtual QRect getWindowGeometry(WId wid) const = 0;

    virtual QString getWindowAppId(WId wid) const = 0;
    virtual QString getWindowTitle(WId wid) const = 0;
    virtual QString getWindowIconName(WId wid) const = 0;
    virtual QByteArray getWindowDesktopFileName(WId wid) const = 0;
    virtual int getWindowPid(WId wid) const = 0;

    virtual bool isSkipTaskbar(WId wid) const = 0;
    virtual bool isMinimized(WId wid) const = 0;
    virtual bool isMaximized(WId wid) const = 0;
    virtual bool isKeepAbove(WId wid) const = 0;
    virtual bool isActive(WId wid) const = 0;
    virtual WId activeWindow() const = 0;

    virtual void closeWindow(WId wid) = 0;
    virtual void activateWindow(WId wid) = 0;
    virtual void minimizeWindow(WId wid) = 0;
    virtual void maximizeWindow(WId wid, bool set) = 0;
    virtual void restoreWindow(WId wid) = 0;
    virtual void setKeepAbove(WId wid, bool set) = 0;
    virtual void moveResizeWindow(WId wid) = 0;

    virtual int numberOfDesktops() const = 0;
    virtual int currentDesktop() const = 0;
    virtual void setCurrentDesktop(int desktop) = 0;
    virtual int getDesktopOfWindow(WId wid) const = 0;
    virtual bool isOnCurrentDesktop(WId wid) const = 0;
    virtual void moveWindowToDesktop(WId wid, int desktop) = 0;
    virtual void createDesktop() = 0;
    virtual void removeDesktop(int deskToRemove = -1) = 0;

    virtual void setWindowSkipTaskbar(WId wid, bool set) = 0;
    virtual QRect workArea(int desktop) const = 0;
    virtual bool isShowingDesktop() const = 0;
    virtual void setShowingDesktop(bool show) = 0;

    virtual QPixmap getWindowIcon(WId wid, const QSize &size) = 0;

    virtual QPixmap getWindowPreview(WId wid) = 0;

signals:
    void windowAdded(WId wid);
    void windowRemoved(WId wid);
    void activeWindowChanged(WId wid);
    void windowTitleChanged(WId wid);
    void windowIconChanged(WId wid);
    void windowStateChanged(WId wid);
    void windowGeometryChanged(WId wid);
    void windowDesktopChanged(WId wid);
    void windowChanged(WId wid);
    void currentDesktopChanged(int desktop);
    void numberOfDesktopsChanged(int num);
};
