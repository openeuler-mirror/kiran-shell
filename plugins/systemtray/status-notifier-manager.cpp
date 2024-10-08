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

#include <QDBusConnection>
#include <QDBusConnectionInterface>

#include "status-notifier-manager.h"
#include "tray-item.h"
#include "window.h"

#define SERVICE_NAME "org.kde.StatusNotifierManager"
#define WATCHER_PATH "/StatusNotifierManager"

StatusNotifierManager::StatusNotifierManager(QObject* parent)
    : QObject{parent}
{
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerService(QStringLiteral(SERVICE_NAME));
    //注册对象，把函数、信号导出为object的method、signal、property
    dbus.registerObject(QStringLiteral(WATCHER_PATH), this, QDBusConnection::ExportAllContents);
}

StatusNotifierManager::~StatusNotifierManager()
{
    QDBusConnection::sessionBus().unregisterService(SERVICE_NAME);
}

QString StatusNotifierManager::GetGeometry(const QString& id)
{
    static const QMap<QString, QString> nameWithId = {
        {"kiran-network-status-icon", "~04-network"},
        {"kiran-audio-status-icon", "~02-volume"}};

    QString name = nameWithId.key(id);
    Kiran::Systemtray::Window* window = (Kiran::Systemtray::Window*)parent();

    for (Kiran::Systemtray::TrayItem* item : window->getTrayItems())
    {
        if (item->getId() == name)
        {
            QRect rect = item->geometry();
            QPoint point = window->mapToGlobal(rect.topLeft());

            QJsonObject jsonObject;
            jsonObject["x"] = point.x();
            jsonObject["y"] = point.y();
            jsonObject["width"] = rect.width();
            jsonObject["height"] = rect.height();
            QJsonDocument jsonDoc(jsonObject);
            QString jsonString = jsonDoc.toJson(QJsonDocument::Indented);
            return jsonString;
        }
    }

    for (Kiran::Systemtray::TrayItem* item : window->getTrayBox()->getTrayItems())
    {
        if (item->getId() == name)
        {
            QRect rect = item->geometry();
            QPoint point = window->getTrayBox()->mapToGlobal(rect.topLeft());

            QJsonObject jsonObject;
            jsonObject["x"] = point.x();
            jsonObject["y"] = point.y();
            jsonObject["width"] = rect.width();
            jsonObject["height"] = rect.height();
            QJsonDocument jsonDoc(jsonObject);
            QString jsonString = jsonDoc.toJson(QJsonDocument::Indented);
            return jsonString;
        }
    }

    return "";
}
