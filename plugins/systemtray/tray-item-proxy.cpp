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

#include "lib/common/logging-category.h"
#include "tray-item-proxy.h"

TrayItemProxy::TrayItemProxy(const QString &service, const QString &path, QObject *parent)
    : QObject(parent), m_statusNotifierItemInterface{service, path, QDBusConnection::sessionBus()}
{
    connect(&m_statusNotifierItemInterface, &StatusNotifierItemInterface::NewIcon, this, &TrayItemProxy::updateIcon);
    connect(&m_statusNotifierItemInterface, &StatusNotifierItemInterface::NewAttentionIcon, this, &TrayItemProxy::updateAttentionIcon);
    connect(&m_statusNotifierItemInterface, &StatusNotifierItemInterface::NewOverlayIcon, this, &TrayItemProxy::updateOverlayIcon);
    connect(&m_statusNotifierItemInterface, &StatusNotifierItemInterface::NewStatus, this, &TrayItemProxy::updateStatus);
    connect(&m_statusNotifierItemInterface, &StatusNotifierItemInterface::NewTitle, this, &TrayItemProxy::updateTitle);
    connect(&m_statusNotifierItemInterface, &StatusNotifierItemInterface::NewToolTip, this, &TrayItemProxy::updateToolTip);
}

QDBusVariant TrayItemProxy::getProperty(const QString &name)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(m_statusNotifierItemInterface.service(), m_statusNotifierItemInterface.path(), QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("Get"));
    msg << m_statusNotifierItemInterface.interface() << name;
    QDBusPendingCall call = m_statusNotifierItemInterface.connection().asyncCall(msg);
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

QStringList TrayItemProxy::getAllPropertyKey()
{
    QDBusMessage msg = QDBusMessage::createMethodCall(m_statusNotifierItemInterface.service(), m_statusNotifierItemInterface.path(), QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("GetAll"));
    msg << m_statusNotifierItemInterface.interface();

    QDBusPendingCall call = m_statusNotifierItemInterface.connection().asyncCall(msg);
    call.waitForFinished();
    const auto arguments = call.reply().arguments();
    if (arguments.isEmpty())
    {
        KLOG_WARNING(LCSystemtray) << "No arguments in the message!" << msg.interface() << msg.path();
        return QStringList();
    }
    QDBusArgument arg = arguments.first().value<QDBusArgument>();
    QVariantMap map;
    arg >> map;
    return map.keys();
}

QString TrayItemProxy::service() const
{
    return m_statusNotifierItemInterface.service();
}

QDBusPendingReply<> TrayItemProxy::activate(int x, int y)
{
    return m_statusNotifierItemInterface.Activate(x, y);
}

QDBusPendingReply<> TrayItemProxy::contextMenu(int x, int y)
{
    return m_statusNotifierItemInterface.ContextMenu(x, y);
}

QDBusPendingReply<> TrayItemProxy::scroll(int delta, const QString &orientation)
{
    return m_statusNotifierItemInterface.Scroll(delta, orientation);
}

QDBusPendingReply<> TrayItemProxy::secondaryActivate(int x, int y)
{
    return m_statusNotifierItemInterface.SecondaryActivate(x, y);
}
