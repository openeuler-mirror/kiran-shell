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
#include <QDBusInterface>

#include "logging-category.h"
#include "notify.h"

#define NOTIFICATION_SERVICE "org.freedesktop.Notifications"
#define NOTIFICATION_PATH "/org/freedesktop/Notifications"
#define NOTIFICATION_INTERFACE "org.freedesktop.Notifications"

namespace Kiran
{
namespace Common
{
void generalNotify(const QString &title, const QString &desc)
{
    QDBusInterface notifyInterface(NOTIFICATION_SERVICE,
                                   NOTIFICATION_PATH,
                                   NOTIFICATION_INTERFACE,
                                   QDBusConnection::sessionBus());
    notifyInterface.setTimeout(500);
    QVariantList args;
    args << "kiran-shell";  // 应用名称
    args << (uint)0;        // 替换现有通知的ID（0表示新通知）
    args << "";             // 图标路径（可选）
    args << title;          // 标题
    args << desc;           // 内容
    args << QStringList();  // 按钮列表（可选）
    args << QVariantMap();  // 附加属性（如紧急程度）
    args << (int)2000;      // 超时时间（毫秒）

    auto reply = notifyInterface.callWithArgumentList(QDBus::AutoDetect, "Notify", args);
    if (reply.type() == QDBusMessage::ErrorMessage)
    {
        KLOG_WARNING(LCLib) << "send notify failed," << reply.errorMessage();
    }
}

}  // namespace Common
}  // namespace Kiran
