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

#include "tray-item-proxy.h"

TrayItemProxy::TrayItemProxy(const QString &service, const QString &path, QObject *parent)
    : QObject(parent), m_statusNotifierItem{service, path, QDBusConnection::sessionBus()}
{
    connect(&m_statusNotifierItem, &org::kde::StatusNotifierItem::NewIcon, this, &TrayItemProxy::updateIcon);
    connect(&m_statusNotifierItem, &org::kde::StatusNotifierItem::NewAttentionIcon, this, &TrayItemProxy::updateAttentionIcon);
    connect(&m_statusNotifierItem, &org::kde::StatusNotifierItem::NewOverlayIcon, this, &TrayItemProxy::updateOverlayIcon);
    connect(&m_statusNotifierItem, &org::kde::StatusNotifierItem::NewStatus, this, &TrayItemProxy::updateStatus);
    connect(&m_statusNotifierItem, &org::kde::StatusNotifierItem::NewTitle, this, &TrayItemProxy::updateTitle);
    connect(&m_statusNotifierItem, &org::kde::StatusNotifierItem::NewToolTip, this, &TrayItemProxy::updateToolTip);
}

QDBusVariant TrayItemProxy::getProperty(const QString &name)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(m_statusNotifierItem.service(), m_statusNotifierItem.path(), QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("Get"));
    msg << m_statusNotifierItem.interface() << name;
    QDBusPendingCall call = m_statusNotifierItem.connection().asyncCall(msg);
    call.waitForFinished();
    if (call.isValid())
    {
        QList<QVariant> data = call.reply().arguments();
        if (!data.isEmpty())
        {
            return qvariant_cast<QDBusVariant>(data.at(0));
        }
    }

    return QDBusVariant();
}

QString TrayItemProxy::service() const
{
    return m_statusNotifierItem.service();
}

QDBusPendingReply<> TrayItemProxy::activate(int x, int y)
{
    return m_statusNotifierItem.Activate(x, y);
}

QDBusPendingReply<> TrayItemProxy::contextMenu(int x, int y)
{
    return m_statusNotifierItem.ContextMenu(x, y);
}

QDBusPendingReply<> TrayItemProxy::scroll(int delta, const QString &orientation)
{
    return m_statusNotifierItem.Scroll(delta, orientation);
}

QDBusPendingReply<> TrayItemProxy::secondaryActivate(int x, int y)
{
    return m_statusNotifierItem.SecondaryActivate(x, y);
}
