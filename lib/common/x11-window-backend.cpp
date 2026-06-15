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

#include "lib/common/app-utils.h"
#include "logging-category.h"
#include "x11-window-backend.h"

namespace Kiran
{
namespace Common
{
static void cleanupXcbImage(void* data)
{
    xcb_image_destroy(static_cast<xcb_image_t*>(data));
}

X11WindowBackend::X11WindowBackend(QObject* parent)
    : WindowManagerBackend(parent)
{
    connect(KWindowSystem::self(), &KWindowSystem::windowAdded, this, &X11WindowBackend::addWindow);
    connect(KWindowSystem::self(), &KWindowSystem::windowRemoved, this, &X11WindowBackend::removeWindow);
    connect(KWindowSystem::self(), &KWindowSystem::activeWindowChanged, this, &X11WindowBackend::changedActiveWindow);
    connect(KWindowSystem::self(),
            QOverload<WId, NET::Properties, NET::Properties2>::of(
                &KWindowSystem::windowChanged),
            this,
            &X11WindowBackend::changedWindow);

    connect(KWindowSystem::self(), &KWindowSystem::currentDesktopChanged, this, [this](int desktop) { emit currentDesktopChanged(desktop); });
    connect(KWindowSystem::self(), &KWindowSystem::numberOfDesktopsChanged, this, [this](int num) { emit numberOfDesktopsChanged(num); });
}

X11WindowBackend::~X11WindowBackend() = default;

QList<WId> X11WindowBackend::getAllWindows() const
{
    return QList<WId>(m_managedWindows.begin(), m_managedWindows.end());
}

QList<WId> X11WindowBackend::getAllWindows(int desktop) const
{
    QList<WId> windows;
    for (auto window : KWindowSystem::stackingOrder())
    {
        KWindowInfo windowInfo(window, NET::WMDesktop);
        if (windowInfo.valid() &&
            windowInfo.desktop() == desktop &&
            !isSkipTaskbar(window))
        {
            windows.append(window);
        }
    }

    return windows;
}

QRect X11WindowBackend::getWindowGeometry(WId wid) const
{
    KWindowInfo info(wid, NET::WMGeometry);
    if (info.valid())
    {
        return info.geometry();
    }

    return {};
}

QString X11WindowBackend::getWindowAppId(WId wid) const
{
    KWindowInfo info(wid, NET::WMPid, NET::WM2WindowClass);
    return info.windowClassName();
}

QString X11WindowBackend::getWindowTitle(WId wid) const
{
    KWindowInfo info(wid, NET::WMVisibleName);
    if (info.valid())
    {
        return info.visibleName();
    }

    KLOG_WARNING(LCLib) << "can't find app name by wid:" << wid;
    return {};
}

QString X11WindowBackend::getWindowIconName(WId wid) const
{
    KWindowInfo info(wid, NET::WMIconName);
    if (info.valid())
    {
        return info.iconName();
    }

    KLOG_WARNING(LCLib) << "can't find app icon by wid:" << wid;
    return {};
}

QByteArray X11WindowBackend::getWindowDesktopFileName(WId wid) const
{
    KWindowInfo info(wid, NET::WMPid, NET::WM2DesktopFileName);
    return info.desktopFileName();
}

int X11WindowBackend::getWindowPid(WId wid) const
{
    KWindowInfo info(wid, NET::WMPid);
    if (info.valid())
    {
        return info.pid();
    }

    return -1;
}

bool X11WindowBackend::isSkipTaskbar(WId wid) const
{
    QFlags<NET::WindowTypeMask> ignoreList;
    ignoreList |= NET::DesktopMask;
    ignoreList |= NET::DockMask;
    ignoreList |= NET::SplashMask;
    ignoreList |= NET::ToolbarMask;
    ignoreList |= NET::MenuMask;
    ignoreList |= NET::PopupMenuMask;
    ignoreList |= NET::NotificationMask;

    KWindowInfo info(wid, NET::WMWindowType | NET::WMState, NET::WM2TransientFor);
    if (!info.valid())
    {
        return true;
    }

    if (NET::typeMatchesMask(info.windowType(NET::AllTypesMask), ignoreList))
    {
        return true;
    }

    return info.hasState(NET::SkipTaskbar) ||
           info.hasState(NET::SkipPager) ||
           info.hasState(NET::SkipSwitcher);
}

bool X11WindowBackend::isMinimized(WId wid) const
{
    KWindowInfo info(wid, NET::WMState | NET::XAWMState);
    if (info.valid())
    {
        return info.isMinimized();
    }

    return false;
}

bool X11WindowBackend::isMaximized(WId wid) const
{
    KWindowInfo info(wid, NET::WMState);
    if (info.valid())
    {
        return info.hasState(NET::Max);
    }

    return false;
}

bool X11WindowBackend::isKeepAbove(WId wid) const
{
    KWindowInfo info(wid, NET::WMState);
    if (info.valid())
    {
        return info.hasState(NET::KeepAbove);
    }

    return false;
}

bool X11WindowBackend::isActive(WId wid) const
{
    return KWindowSystem::activeWindow() == wid;
}

WId X11WindowBackend::activeWindow() const
{
    return KWindowSystem::activeWindow();
}

void X11WindowBackend::closeWindow(WId wid)
{
    NETRootInfo netRootInfo(QX11Info::connection(), NET::CloseWindow);
    netRootInfo.closeWindowRequest(wid);
}

void X11WindowBackend::activateWindow(WId wid)
{
    KWindowSystem::activateWindow(wid);
}

void X11WindowBackend::minimizeWindow(WId wid)
{
    KWindowSystem::minimizeWindow(wid);
}

void X11WindowBackend::maximizeWindow(WId wid, bool set)
{
    if (set)
    {
        KWindowSystem::setState(wid, NET::Max);
    }
    else
    {
        KWindowSystem::clearState(wid, NET::Max);
    }

    KWindowSystem::activateWindow(wid);
}

void X11WindowBackend::restoreWindow(WId wid)
{
    KWindowSystem::clearState(wid, NET::Max);
}

void X11WindowBackend::setKeepAbove(WId wid, bool set)
{
    if (set)
    {
        KWindowSystem::setState(wid, NET::KeepAbove);
    }
    else
    {
        KWindowSystem::clearState(wid, NET::KeepAbove);
    }
}

void X11WindowBackend::moveResizeWindow(WId wid)
{
    QRect rect = getWindowGeometry(wid);

    NETRootInfo netRootInfo(QX11Info::connection(), NET::WMMoveResize);
    netRootInfo.moveResizeRequest(wid, rect.center().x(), rect.center().y(), NET::KeyboardMove);
}

int X11WindowBackend::numberOfDesktops() const
{
    return KWindowSystem::numberOfDesktops();
}

int X11WindowBackend::currentDesktop() const
{
    return KWindowSystem::currentDesktop();
}

void X11WindowBackend::setCurrentDesktop(int desktop)
{
    KWindowSystem::setCurrentDesktop(desktop);
}

int X11WindowBackend::getDesktopOfWindow(WId wid) const
{
    KWindowInfo info(wid, NET::WMDesktop);
    if (info.valid())
    {
        return info.desktop();
    }

    return 0;
}

bool X11WindowBackend::isOnCurrentDesktop(WId wid) const
{
    KWindowInfo info(wid, NET::WMDesktop);
    if (info.valid())
    {
        return info.isOnCurrentDesktop();
    }

    return false;
}

void X11WindowBackend::moveWindowToDesktop(WId wid, int desktop)
{
    KWindowSystem::setOnDesktop(wid, desktop);
}

void X11WindowBackend::createDesktop()
{
    NETRootInfo info(QX11Info::connection(), NET::NumberOfDesktops);
    info.setNumberOfDesktops(numberOfDesktops() + 1);
}

void X11WindowBackend::removeDesktop(int deskToRemove)
{
    int numOfDesk = numberOfDesktops();
    if (numOfDesk <= 1 || numOfDesk < deskToRemove || deskToRemove < 1)
    {
        return;
    }

    QMap<int, QList<WId>> winWithDesk;
    for (int i = 1; i <= numOfDesk; i++)
    {
        winWithDesk[i] = QList<WId>{};
    }
    QList<WId> windows = KWindowSystem::windows();
    for (auto window : windows)
    {
        KWindowInfo windowInfo(window, NET::WMDesktop);
        if (windowInfo.valid())
        {
            winWithDesk[windowInfo.desktop()].append(window);
        }
    }

    for (int i = deskToRemove; i <= numOfDesk; i++)
    {
        if (i == 1)
        {
            i++;
            continue;
        }

        for (auto window : winWithDesk[i])
        {
            KWindowSystem::setOnDesktop(window, i - 1);
        }
    }

    NETRootInfo info(QX11Info::connection(), NET::NumberOfDesktops);
    info.setNumberOfDesktops(info.numberOfDesktops() - 1);
}

void X11WindowBackend::setWindowSkipTaskbar(WId wid, bool set)
{
    if (set)
        KWindowSystem::setState(wid, NET::SkipTaskbar | NET::SkipPager | NET::SkipSwitcher);
    else
        KWindowSystem::clearState(wid, NET::SkipTaskbar | NET::SkipPager | NET::SkipSwitcher);
}

QRect X11WindowBackend::workArea(int desktop) const
{
    return KWindowSystem::workArea(desktop);
}

bool X11WindowBackend::isShowingDesktop() const
{
    return KWindowSystem::showingDesktop();
}

void X11WindowBackend::setShowingDesktop(bool show)
{
    KWindowSystem::setShowingDesktop(show);
}

QPixmap X11WindowBackend::getWindowIcon(WId wid, const QSize &size)
{
    return KWindowSystem::icon(wid, size.width(), size.height(), true);
}

QPixmap X11WindowBackend::getWindowPreview(WId wid)
{
    if (!m_managedWindows.contains(wid))
    {
        return {};
    }

    QRect rect = getWindowGeometry(wid);
    if (rect.width() <= 0 || rect.height() <= 0)
    {
        return {};
    }

    xcb_image_t* image = xcb_image_get(QX11Info::connection(), wid, 0, 0, rect.width(), rect.height(), ~0, XCB_IMAGE_FORMAT_Z_PIXMAP);
    if (image)
    {
        QImage normalImage = x11ImageToQimage(image);
        return QPixmap::fromImage(normalImage);
    }

    return generatePreviewByIcon(wid, rect.size());
}

void X11WindowBackend::addWindow(WId wid)
{
    if (m_managedWindows.contains(wid))
    {
        return;
    }
    if (!isSkipTaskbar(wid))
    {
        m_managedWindows.insert(wid);
        emit windowAdded(wid);
    }
}

void X11WindowBackend::removeWindow(WId wid)
{
    if (m_managedWindows.contains(wid))
    {
        m_managedWindows.remove(wid);
        emit windowRemoved(wid);
    }
}

void X11WindowBackend::changedActiveWindow(WId wid)
{
    if (!isSkipTaskbar(wid))
    {
        if (m_managedWindows.contains(wid))
        {
            emit activeWindowChanged(wid);
        }
    }
}

void X11WindowBackend::changedWindow(WId wid, NET::Properties properties, NET::Properties2 properties2)
{
    Q_UNUSED(properties2);

    if (isSkipTaskbar(wid))
    {
        if (m_managedWindows.contains(wid))
        {
            removeWindow(wid);
        }
    }
    else
    {
        if (!m_managedWindows.contains(wid))
        {
            addWindow(wid);
        }
    }

    if (m_managedWindows.contains(wid))
    {
        if (properties & NET::WMName || properties & NET::WMVisibleName)
        {
            emit windowTitleChanged(wid);
        }
        if (properties & NET::WMIconName)
        {
            emit windowIconChanged(wid);
        }
        if (properties & NET::WMState)
        {
            emit windowStateChanged(wid);
        }
        if (properties & NET::WMGeometry)
        {
            emit windowGeometryChanged(wid);
        }
        if (properties.testFlag(NET::WMDesktop))
        {
            emit windowDesktopChanged(wid);
        }

        emit windowChanged(wid);
    }
}

QImage X11WindowBackend::x11ImageToQimage(xcb_image_t* xcbImage)
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
        auto* pixels = reinterpret_cast<quint32*>(xcbImage->data);
        for (uint i = 0; i < (xcbImage->size / 4); i++)
        {
            int r = (pixels[i] >> 22) & 0xff;
            int g = (pixels[i] >> 12) & 0xff;
            int b = (pixels[i] >> 2) & 0xff;

            pixels[i] = qRgba(r, g, b, 0xff);
        }
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

    if (image.format() == QImage::Format_MonoLSB)
    {
        image.setColorCount(2);
        image.setColor(0, QColor(Qt::white).rgb());
        image.setColor(1, QColor(Qt::black).rgb());
    }

    return image;
}

QPixmap X11WindowBackend::generatePreviewByIcon(WId wid, const QSize &size)
{
    QPixmap pix(size);
    pix.fill(Qt::transparent);

    QPainter painter(&pix);
    QColor semiTransparentColor(0, 0, 0, 128);
    painter.setBrush(QBrush(semiTransparentColor));
    painter.drawRect(pix.rect());

    QPixmap iconPix = Kiran::getWindowAppIcon(wid, QSize(100, 100));
    iconPix = iconPix.scaled(QSize(100, 100), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    int centerX = (pix.width() - iconPix.width()) / 2;
    int centerY = (pix.height() - iconPix.height()) / 2;
    painter.drawPixmap(centerX, centerY, iconPix);

    painter.end();

    return pix;
}
}  // namespace Common
}  // namespace Kiran
