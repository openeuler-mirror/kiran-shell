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
#include <QPixmap>
#include <QRect>
#include <QSize>
#include <QString>

#include "window-manager-backend.h"

namespace Kiran
{
namespace Common
{
class WaylandWindowBackend : public WindowManagerBackend
{
    Q_OBJECT
public:
    explicit WaylandWindowBackend(QObject* parent = nullptr);
    ~WaylandWindowBackend() override;

    QList<WId> getAllWindows() const override;
    QList<WId> getAllWindows(int desktop) const override;
    QRect getWindowGeometry(WId wid) const override;
    QString getWindowAppId(WId wid) const override;
    QString getWindowTitle(WId wid) const override;
    QString getWindowIconName(WId wid) const override;
    QByteArray getWindowDesktopFileName(WId wid) const override;
    int getWindowPid(WId wid) const override;
    bool isSkipTaskbar(WId wid) const override;
    bool isMinimized(WId wid) const override;
    bool isMaximized(WId wid) const override;
    bool isKeepAbove(WId wid) const override;
    bool isActive(WId wid) const override;
    WId activeWindow() const override;
    void closeWindow(WId wid) override;
    void activateWindow(WId wid) override;
    void minimizeWindow(WId wid) override;
    void maximizeWindow(WId wid, bool set) override;
    void restoreWindow(WId wid) override;
    void setKeepAbove(WId wid, bool set) override;
    void moveResizeWindow(WId wid) override;
    int numberOfDesktops() const override;
    int currentDesktop() const override;
    void setCurrentDesktop(int desktop) override;
    int getDesktopOfWindow(WId wid) const override;
    bool isOnCurrentDesktop(WId wid) const override;
    void moveWindowToDesktop(WId wid, int desktop) override;
    void createDesktop() override;
    void removeDesktop(int deskToRemove = -1) override;
    void setWindowSkipTaskbar(WId wid, bool set) override;
    QRect workArea(int desktop) const override;
    bool isShowingDesktop() const override;
    void setShowingDesktop(bool show) override;
    QPixmap getWindowIcon(WId wid, const QSize &size) override;
    QPixmap getWindowPreview(WId wid) override;
};
}  // namespace Common
}  // namespace Kiran
