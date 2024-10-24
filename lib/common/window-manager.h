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

#include <qwindowdefs.h>
#include <KWindowSystem>
#include <QMap>
#include <QObject>
#include <QPixmap>

namespace Kiran
{
namespace Common
{
class Window : public QObject
{
    Q_OBJECT
public:
    Window(WId wid, QObject* parent = nullptr);
    ~Window();

    QRect getWindowGeometry();
    QPixmap getPixPreviewr();

    // 启动截图
    void startUpdatePreviewer();

private:
    // 获取截图，更新显示
    void updatePreviewer();
    // 没有获取到截图,绘制一个透明背景,中心绘制一个图标
    void updatePreviewerByIcon();

signals:
    void previewrUpdated(WId wid);

private:
    WId m_wid;

    // 应用截图
    bool m_updateInProgress = false;
    QPixmap m_pixPreviewer;
};

class WindowManager : public QObject
{
    Q_OBJECT
public:
    static WindowManager& getInstance();

    QList<WId> getAllWindow();
    QList<WId> getAllWindow(int desktop);

    QRect getWindowGeometry(WId wid);
    QPixmap getPixPreviewr(WId wid);

private:
    WindowManager();
    ~WindowManager();

    //  打开或关闭窗口软件
    void addWindow(WId wid);
    void removeWindow(WId wid);
    // 监测窗口变化
    void changedActiveWindow(WId wid);
    void changedWindow(WId wid, NET::Properties properties, NET::Properties2 properties2);

signals:
    void windowAdded(WId wid);
    void windowRemoved(WId wid);
    void activeWindowChanged(WId wid);
    void windowChanged(WId, NET::Properties, NET::Properties2);

    void previewrUpdated(WId wid);
    void visibleNameUpdated(WId wid);

private:
    QMap<WId, Window*> m_windows;
};

}  // namespace Common
}  // namespace Kiran

#define WindowManagerInstance Kiran::Common::WindowManager::getInstance()
#define WindowManagerInit Kiran::Common::WindowManager::getInstance()
