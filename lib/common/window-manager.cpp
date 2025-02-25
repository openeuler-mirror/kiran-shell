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

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <private/qtx11extras_p.h>
#else
#include <QX11Info>
#endif

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

Window::~Window() = default;

QRect Window::getWindowGeometry() const
{
    KWindowInfo info(m_wid, NET::WMGeometry);
    if (info.valid())
    {
        return info.geometry();
    }

    return {};
}

QPixmap Window::getPixPreviewr()
{
    updatePreviewer();

    return m_pixPreviewer;
}

static void cleanupXcbImage(void* data)
{
    xcb_image_destroy(static_cast<xcb_image_t*>(data));
}

void Window::updatePreviewer()
{
    QRect rect = getWindowGeometry();
    if (rect.width() <= 0 || rect.height() <= 0)
    {
        return;
    }

    // TODO: wayland 窗口截图

    // X11 窗口截图
    xcb_image_t* image = xcb_image_get(QX11Info::connection(), m_wid, 0, 0, rect.width(), rect.height(), ~0, XCB_IMAGE_FORMAT_Z_PIXMAP);
    if (image)
    {
        QImage normalImage = x11ImageToQimage(image);
        m_pixPreviewer = QPixmap::fromImage(normalImage);
    }

    if (m_pixPreviewer.isNull())
    {
        updatePreviewerByIcon();
    }
}

QImage Window::x11ImageToQimage(xcb_image_t* xcbImage)
{
    QImage::Format format = QImage::Format_Invalid;

    switch (xcbImage->depth)
    {
    case 1:
        format = QImage::Format_MonoLSB;
        break;
    case 16:
        format = QImage::Format_RGB16;
        break;
    case 24:
        format = QImage::Format_RGB32;
        break;
    case 30:
    {
        // Qt doesn't have a matching image format. We need to convert manually
        auto* pixels = reinterpret_cast<quint32*>(xcbImage->data);
        for (uint i = 0; i < (xcbImage->size / 4); i++)
        {
            int r = (pixels[i] >> 22) & 0xff;
            int g = (pixels[i] >> 12) & 0xff;
            int b = (pixels[i] >> 2) & 0xff;

            pixels[i] = qRgba(r, g, b, 0xff);
        }
        // fall through, Qt format is still Format_ARGB32_Premultiplied
        Q_FALLTHROUGH();
    }
    case 32:
        format = QImage::Format_ARGB32_Premultiplied;
        break;
    default:
        return {};
    }

    QImage image(xcbImage->data, xcbImage->width, xcbImage->height, xcbImage->stride, format, cleanupXcbImage, xcbImage);

    if (image.isNull())
    {
        return {};
    }

    // 黑白图
    // work around an abort in QImage::color
    if (image.format() == QImage::Format_MonoLSB)
    {
        image.setColorCount(2);
        image.setColor(0, QColor(Qt::white).rgb());
        image.setColor(1, QColor(Qt::black).rgb());
    }

    return image;
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
    connect(KWindowSystem::self(), &KWindowSystem::windowAdded, this, &WindowManager::addWindow);
    connect(KWindowSystem::self(), &KWindowSystem::windowRemoved, this, &WindowManager::removeWindow);

    connect(KWindowSystem::self(), &KWindowSystem::activeWindowChanged, this, &WindowManager::changedActiveWindow);

    connect(KWindowSystem::self(),
            QOverload<WId, NET::Properties, NET::Properties2>::of(
                &KWindowSystem::windowChanged),
            this,
            &WindowManager::changedWindow);
}

WindowManager::~WindowManager() = default;

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

    return {};
}

QPixmap WindowManager::getPixPreviewr(WId wid)
{
    if (m_windows.contains(wid))
    {
        return m_windows[wid]->getPixPreviewr();
    }

    return {};
}
void WindowManager::addWindow(WId wid)
{
    if (!WindowInfoHelper::isSkipTaskbar(wid))
    {
        auto* window = new Window(wid, this);
        m_windows[wid] = window;
        emit windowAdded(wid);
    }
}

void WindowManager::removeWindow(WId wid)
{
    if (!WindowInfoHelper::isSkipTaskbar(wid))
    {
        if (m_windows.contains(wid))
        {
            auto* window = m_windows.take(wid);
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
            emit activeWindowChanged(wid);
        }
    }
}

void WindowManager::changedWindow(WId wid, NET::Properties properties, NET::Properties2 properties2)
{
    emit windowChanged(wid, properties, properties2);
}

}  // namespace Common
}  // namespace Kiran
