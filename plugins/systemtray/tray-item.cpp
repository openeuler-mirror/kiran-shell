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

#include <netinet/in.h>
#include <qt5-log-i.h>
#include <QDrag>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>

#include "lib/common/logging-category.h"
#include "lib/common/tray-data-types.h"
#include "tray-item.h"

namespace Kiran
{
namespace Systemtray
{
TrayItem::TrayItem(QString service, QString objectPath, QWidget *parent)
    : StyledButton(parent)
{
    KLOG_INFO(LCSystemtray) << "trat item create" << service << objectPath;
    m_service = service;
    m_objectPath = objectPath;

    m_trayItemProxy = new TrayItemProxy(m_service, m_objectPath, this);

    // 延后执行刷新
    m_refreshTimer = new QTimer(this);
    m_refreshTimer->setSingleShot(true);
    connect(m_refreshTimer, &QTimer::timeout, this, &TrayItem::refresh);
    performRefresh();
}

TrayItem::~TrayItem()
{
}

void TrayItem::updateBaseIcon()
{
    getIcon(BASE_ICON);
    updateIconShow();
}

void TrayItem::updateAttentionIcon()
{
    getIcon(ATTENTION_ICON);
    updateIconShow();
}

void TrayItem::updateOverlayIcon()
{
    getIcon(OVERLAY_ICON);
    updateIconShow();
}

void TrayItem::updateToolTip()
{
    ToolTip tooltip;
    QDBusVariant busdata;

    if (m_propertyKeys.contains("ToolTip"))
    {
        busdata = m_trayItemProxy->getProperty("ToolTip");
        busdata.variant().value<QDBusArgument>() >> tooltip;
    }

    QString toolTipTitle = tooltip.title;
    if (!toolTipTitle.isEmpty())
    {
        setToolTip(toolTipTitle);
    }
    else
    {
        if (m_propertyKeys.contains("Title"))
        {
            busdata = m_trayItemProxy->getProperty(QLatin1String("Title"));
            QString title = busdata.variant().toString();
            if (!title.isEmpty())
            {
                setToolTip(title);
            }
        }
    }
}

void TrayItem::updateStatus(const QString &status)
{
    Status newStatus;
    if (status == QLatin1String("Passive"))
        newStatus = PASSIVE;
    else if (status == QLatin1String("NeedsAttention"))
        newStatus = NEEDSATTENTION;
    else
        newStatus = ACTIVE;

    m_status = newStatus;
    updateIconShow();
}

void TrayItem::updataItemMenu()
{
    QMenu *menu = m_dBusMenuImporter->menu();
    if (menu && !menu->isEmpty())
    {
        menu->exec(m_dBusMenuImporter->menu()->actions(), QCursor::pos(), nullptr, this);  // 任务栏显示右键菜单
    }
    else
    {
        m_trayItemProxy->contextMenu(QCursor::pos().x(), QCursor::pos().y());  // 应用显示右键菜单
    }
}

void TrayItem::contextMenuEvent(QContextMenuEvent *event)
{
    if (m_dBusMenuImporter)
    {
        m_dBusMenuImporter->updateMenu();
    }
    else
    {
        m_trayItemProxy->contextMenu(QCursor::pos().x(), QCursor::pos().y());
    }

    m_hovered = false;
    m_pressed = false;
    update();
}

void TrayItem::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        m_pressPoint = event->pos();
    }
}

void TrayItem::mouseReleaseEvent(QMouseEvent *event)
{
    //    KLOG_INFO(LCSystemtray) << "TrayItem::mouseReleaseEvent" << QCursor::pos().x() << QCursor::pos().y();
    if (event->button() == Qt::LeftButton)
    {
        m_trayItemProxy->activate(QCursor::pos().x(), QCursor::pos().y());
    }
    else if (event->button() == Qt::MidButton)
    {
        m_trayItemProxy->secondaryActivate(QCursor::pos().x(), QCursor::pos().y());
    }

    update();
    QToolButton::mouseReleaseEvent(event);
}

void TrayItem::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        if (qAbs(event->pos().x() - m_pressPoint.x()) < 10 && qAbs(event->pos().y() - m_pressPoint.y()) < 10)
        {
            return;
        }

        emit startDrag(this);

        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;
        QByteArray ba;
        ba.append(m_service.toLocal8Bit());
        ba.append(m_objectPath.toLocal8Bit());
        mimeData->setData("serviceAndPath", ba);
        drag->setMimeData(mimeData);
        drag->setPixmap(icon().pixmap(40, 40));
        drag->exec(Qt::CopyAction);
    }
}

void TrayItem::performRefresh()
{
    m_refreshTimer->start(100);
}

void TrayItem::refresh()
{
    // NOTE: 当前仅初始化需要延后执行
    if (!m_isInit)
    {
        init();
    }
}

void TrayItem::init()
{
    connect(m_trayItemProxy, &TrayItemProxy::updateIcon, this, &TrayItem::updateBaseIcon);
    connect(m_trayItemProxy, &TrayItemProxy::updateOverlayIcon, this, &TrayItem::updateOverlayIcon);
    connect(m_trayItemProxy, &TrayItemProxy::updateAttentionIcon, this, &TrayItem::updateAttentionIcon);
    connect(m_trayItemProxy, &TrayItemProxy::updateToolTip, this, &TrayItem::updateToolTip);
    connect(m_trayItemProxy, &TrayItemProxy::updateStatus, this, &TrayItem::updateStatus);

    m_propertyKeys = m_trayItemProxy->getAllPropertyKey();

    QDBusVariant busdata;

    // 菜单
    if (m_propertyKeys.contains("Menu"))
    {
        busdata = m_trayItemProxy->getProperty(QLatin1String("Menu"));
        QDBusObjectPath path = busdata.variant().value<QDBusObjectPath>();
        if (path.path() != QLatin1String("/NO_DBUSMENU") && !path.path().isEmpty())
        {
            m_dBusMenuImporter = new DBusMenuImporter(m_trayItemProxy->service(), path.path(), this);
            if (m_dBusMenuImporter)
            {
                connect(m_dBusMenuImporter, &DBusMenuImporter::menuUpdated, this, &TrayItem::updataItemMenu);
            }
        }
    }

    // 图标
    // 标准中不存在，但Qt和KDE实现中有
    if (m_propertyKeys.contains("IconThemePath"))
    {
        busdata = m_trayItemProxy->getProperty(QLatin1String("IconThemePath"));
        m_iconThemePath = busdata.variant().toString();
    }

    getIcon(BASE_ICON);
    getIcon(ATTENTION_ICON);
    getIcon(OVERLAY_ICON);

    // 状态
    if (m_propertyKeys.contains("Status"))
    {
        busdata = m_trayItemProxy->getProperty(QLatin1String("Status"));
        QString status = busdata.variant().toString();
        if (!status.isEmpty())
        {
            updateStatus(status);
        }
        else
        {
            updateStatus(QLatin1String("Active"));
        }
    }

    // 提示信息
    updateToolTip();

    m_isInit = true;
}

void TrayItem::getIcon(IconType iconType)
{
    QIcon *icon;

    QString nameProperty;
    QString pixmapProperty;
    switch (iconType)
    {
    case ATTENTION_ICON:
        nameProperty = QLatin1String("AttentionIconName");
        pixmapProperty = QLatin1String("AttentionIconPixmap");
        m_attentionIcon = QIcon();
        icon = &m_attentionIcon;
        break;
    case OVERLAY_ICON:
        nameProperty = QLatin1String("OverlayIconName");
        pixmapProperty = QLatin1String("OverlayIconPixmap");
        m_overlayIcon = QIcon();
        icon = &m_overlayIcon;
        break;
    default:
        nameProperty = QLatin1String("IconName");
        pixmapProperty = QLatin1String("IconPixmap");
        m_icon = QIcon();
        icon = &m_icon;
        break;
    }

    // 优先使用图标名称
    // 若没有，再获取图片二进制
    QString iconName;
    if (m_propertyKeys.contains(nameProperty))
    {
        QDBusVariant busdata = m_trayItemProxy->getProperty(nameProperty);
        iconName = busdata.variant().toString();
    }

    if (!iconName.isEmpty())
    {
        if (QIcon::hasThemeIcon(iconName))
        {
            *icon = QIcon::fromTheme(iconName);
        }
        else if (QDir(m_iconThemePath).exists())
        {
            QDirIterator it(m_iconThemePath, QStringList() << iconName + ".png", QDir::Files, QDirIterator::Subdirectories);
            while (it.hasNext())
            {
                icon->addFile(it.filePath());
            }
        }
    }
    else
    {
        IconPixmapVector iconPixmaps;

        if (m_propertyKeys.contains(pixmapProperty))
        {
            QDBusVariant busdata = m_trayItemProxy->getProperty(pixmapProperty);
            busdata.variant().value<QDBusArgument>() >> iconPixmaps;
        }

        if (iconPixmaps.empty())
        {
            return;
        }

        for (IconPixmap iconPixmap : iconPixmaps)
        {
            if (!iconPixmap.bytes.isNull())
            {
                // 网络字节序转本地字节序
                if (QSysInfo::ByteOrder == QSysInfo::LittleEndian)
                {
                    uint *unitData = (uint *)iconPixmap.bytes.data();
                    for (uint i = 0; i < iconPixmap.bytes.size() / sizeof(uint); ++i)
                    {
                        *unitData = ntohl(*unitData);
                        ++unitData;
                    }
                }

                QImage image((uchar *)iconPixmap.bytes.data(), iconPixmap.width, iconPixmap.height, QImage::Format_ARGB32);
                icon->addPixmap(QPixmap::fromImage(image));
            }
        }
    }
}

QIcon TrayItem::mergeIcons(const QIcon &icon1, const QIcon &icon2)
{
    QPixmap combinedPixmap(iconSize());
    combinedPixmap.fill(Qt::transparent);

    QPainter painter(&combinedPixmap);
    painter.drawPixmap(0, 0, icon1.pixmap(iconSize()));
    painter.drawPixmap(0, 0, icon2.pixmap(iconSize()));
    painter.end();

    return QIcon(combinedPixmap);
}

void TrayItem::updateIconShow()
{
    switch (m_status)
    {
    case PASSIVE:
    {
        setVisible(false);
        return;
    }
    case NEEDSATTENTION:
    {
        if (!m_overlayIcon.isNull() && !m_attentionIcon.isNull())
        {
            QIcon icon = mergeIcons(m_attentionIcon, m_overlayIcon);
            setIcon(icon);
        }
        else if (!m_attentionIcon.isNull())
        {
            setIcon(m_attentionIcon);
        }
        break;
    }
    default:
    {
        if (!m_overlayIcon.isNull() && !m_icon.isNull())
        {
            QIcon icon = mergeIcons(m_icon, m_overlayIcon);
            setIcon(icon);
        }
        else if (!m_icon.isNull())
        {
            setIcon(m_icon);
        }
        break;
    }
    }

    setVisible(true);
}

}  // namespace Systemtray
}  // namespace Kiran
