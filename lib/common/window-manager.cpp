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
#include <KWindowSystem>
#include <QGuiApplication>
#include <QPainter>
#include <QScreen>
#include <QTimer>

#include "window-info-helper.h"
#include "window-manager.h"

namespace Kiran
{
namespace Common
{
Window::Window(WId wid, QObject* parent)
    : QObject(parent), m_wid(wid)
{
}

Window::~Window()
{
}

QRect Window::getWindowGeometry()
{
    KWindowInfo info(m_wid, NET::WMGeometry);
    if (info.valid())
    {
        return info.geometry();
    }

    return QRect();
}

QPixmap Window::getPixPreviewr()
{
    if (m_pixPreviewer.isNull())
    {
        updatePreviewerByIcon();
    }
    return m_pixPreviewer;
}

void Window::startUpdatePreviewer()
{
    if (!m_updateInProgress)
    {
        // 需要延迟处理：标志比窗口更新快，当标志完成更新时，窗口还没有更新完成
        // 避免短时间内多次调用
        m_updateInProgress = true;
        QTimer::singleShot(200, this, [this]()
                           {
                               updatePreviewer();
                           });
    }
}

void Window::updatePreviewer()
{
    //    KLOG_INFO() << "Window::updatePreviewer" << m_wid;
    QScreen* screen = QGuiApplication::primaryScreen();
    m_pixPreviewer = screen->grabWindow(m_wid);
    if (m_pixPreviewer.isNull())
    {
        updatePreviewerByIcon();
    }

    m_updateInProgress = false;

    emit previewrUpdated(m_wid);
}

void Window::updatePreviewerByIcon()
{
    QRect rect = getWindowGeometry();
    if (rect.width() <= 0 || rect.height() <= 0)
    {
        return;
    }

    m_pixPreviewer = QPixmap(rect.size());

    m_pixPreviewer.fill(Qt::transparent);  // 初始化为透明背景

    // 使用 QPainter 在 QPixmap 上绘制
    QPainter painter(&m_pixPreviewer);
    QColor semiTransparentColor(0, 0, 0, 128);  // 50% 透明
    painter.setBrush(QBrush(semiTransparentColor));
    painter.drawRect(m_pixPreviewer.rect());

    QPixmap iconPix = KWindowSystem::icon(m_wid);
    QSize smallSize(100, 100);  // 设置小图片的尺寸
    iconPix = iconPix.scaled(smallSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // 计算中心位置
    int centerX = (m_pixPreviewer.width() - iconPix.width()) / 2;
    int centerY = (m_pixPreviewer.height() - iconPix.height()) / 2;
    // 在中心位置绘制icon
    painter.drawPixmap(centerX, centerY, iconPix);

    painter.end();
}

WindowManager::WindowManager()
{
    //    KLOG_INFO() << "WindowManager::WindowManager";

    connect(KWindowSystem::self(), &KWindowSystem::windowAdded, this, &WindowManager::addWindow);
    connect(KWindowSystem::self(), &KWindowSystem::windowRemoved, this, &WindowManager::removeWindow);

    connect(KWindowSystem::self(), &KWindowSystem::activeWindowChanged, this, &WindowManager::changedActiveWindow);

    connect(KWindowSystem::self(),
            QOverload<WId, NET::Properties, NET::Properties2>::of(
                &KWindowSystem::windowChanged),
            this,
            &WindowManager::changedWindow);
}

WindowManager::~WindowManager()
{
}

WindowManager& WindowManager::getInstance()
{
    static WindowManager instance;
    return instance;
}

QList<WId> WindowManager::getAllWindow()
{
    return m_windows.keys();
}

QList<WId> WindowManager::getAllWindow(int desktop)
{
    // 通过desktop获取窗口列表,提供排序过的窗口列表
    QList<WId> windows;
    for (auto window : KWindowSystem::stackingOrder())
    {
        KWindowInfo windowInfo(window, NET::WMDesktop);
        if (windowInfo.valid() &&
            windowInfo.desktop() == desktop &&
            !WindowInfoHelper::isSkipTaskbar(window))
        {
            windows.append(window);
        }
    }

    return windows;
}

QRect WindowManager::getWindowGeometry(WId wid)
{
    if (m_windows.contains(wid))
    {
        return m_windows[wid]->getWindowGeometry();
    }

    return QRect();
}

QPixmap WindowManager::getPixPreviewr(WId wid)
{
    if (m_windows.contains(wid))
    {
        return m_windows[wid]->getPixPreviewr();
    }

    return QPixmap();
}
void WindowManager::addWindow(WId wid)
{
    if (!WindowInfoHelper::isSkipTaskbar(wid))
    {
        auto window = new Window(wid, this);
        m_windows[wid] = window;
        emit windowAdded(wid);

        connect(window, &Window::previewrUpdated, this, &WindowManager::previewrUpdated);
    }
}

void WindowManager::removeWindow(WId wid)
{
    if (!WindowInfoHelper::isSkipTaskbar(wid))
    {
        if (m_windows.contains(wid))
        {
            auto window = m_windows.take(wid);
            if (window)
            {
                delete window;
                window = nullptr;
            }

            emit windowRemoved(wid);
        }
    }
}

void WindowManager::changedActiveWindow(WId wid)
{
    if (!WindowInfoHelper::isSkipTaskbar(wid))
    {
        if (m_windows.contains(wid))
        {
            m_windows[wid]->startUpdatePreviewer();
            emit activeWindowChanged(wid);
        }
    }
}

void WindowManager::changedWindow(WId wid, NET::Properties properties, NET::Properties2 properties2)
{
    if (m_windows.contains(wid))
    {
        // 窗口位置变化
        // KLOG_INFO() << "WindowPreviewer::changedWindow" << properties << properties2;
        if (properties.testFlag(NET::WMGeometry))
        {
            m_windows[wid]->startUpdatePreviewer();
        }
    }

    emit windowChanged(wid, properties, properties2);
}

}  // namespace Common
}  // namespace Kiran
