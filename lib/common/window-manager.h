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

#include <QByteArray>
#include <QObject>
#include <QPixmap>
#include <QRect>
#include <QScopedPointer>
#include <QSize>
#include <QString>
#include <QWidgetList>

class WindowManagerBackend;

namespace Kiran
{
namespace Common
{
class WindowManager : public QObject
{
    Q_OBJECT
public:
    static WindowManager& getInstance();

    // 旧有公共接口，保持兼容
    QList<WId> getAllWindow();
    QList<WId> getAllWindow(int desktop);
    QRect getWindowGeometry(WId wid);
    QPixmap getPixPreviewr(WId wid);

    // 后端提供的新公共接口
    QString getWindowAppId(WId wid) const;
    QString getWindowTitle(WId wid) const;
    QString getWindowIconName(WId wid) const;
    QByteArray getWindowDesktopFileName(WId wid) const;
    int getWindowPid(WId wid) const;
    bool isSkipTaskbar(WId wid) const;
    bool isMinimized(WId wid) const;
    bool isMaximized(WId wid) const;
    bool isKeepAbove(WId wid) const;
    bool isActive(WId wid) const;
    WId activeWindow() const;
    void closeWindow(WId wid);
    void activateWindow(WId wid);
    void minimizeWindow(WId wid);
    void maximizeWindow(WId wid, bool set);
    void restoreWindow(WId wid);
    void setKeepAbove(WId wid, bool set);
    void moveResizeWindow(WId wid);
    int numberOfDesktops() const;
    int currentDesktop() const;
    void setCurrentDesktop(int desktop);
    int getDesktopOfWindow(WId wid) const;
    bool isOnCurrentDesktop(WId wid) const;
    void moveWindowToDesktop(WId wid, int desktop);
    void createDesktop();
    void removeDesktop(int deskToRemove = -1);
    void setWindowSkipTaskbar(WId wid, bool set);
    QRect workArea(int desktop) const;
    bool isShowingDesktop() const;
    void setShowingDesktop(bool show);
    QPixmap getWindowIcon(WId wid, const QSize &size);
    QPixmap getWindowPreview(WId wid);

private:
    WindowManager();
    ~WindowManager() override;

signals:
    // 旧有信号，保持兼容
    void windowAdded(WId wid);
    void windowRemoved(WId wid);
    void activeWindowChanged(WId wid);
    void windowChanged(WId wid);

    // 后端提供的新信号
    void windowTitleChanged(WId wid);
    void windowIconChanged(WId wid);
    void windowStateChanged(WId wid);
    void windowGeometryChanged(WId wid);
    void windowDesktopChanged(WId wid);
    void currentDesktopChanged(int desktop);
    void numberOfDesktopsChanged(int num);

private:
    QScopedPointer<WindowManagerBackend> m_backend;
};

}  // namespace Common
}  // namespace Kiran

#define WindowManagerInstance Kiran::Common::WindowManager::getInstance()
#define WindowManagerInit Kiran::Common::WindowManager::getInstance()
