/**
 * Copyright (c) 2024 ~ 2025 KylinSec Co., Ltd.
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

#include <NetworkManagerQt/WirelessDevice>
#include <QTreeWidget>

#include "net-common.h"

// 连接树，分为有线和无线
// 管理设备项、连接项

namespace Kiran
{
namespace SettingBar
{
class DeviceWidget;
class WiredConnectionWidget;
class WirelessConnectionWidget;

class netTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    netTreeWidget(NetworkManager::Device::Type deviceType, QWidget *parent = nullptr);

    void updateNetworkStatus();

private:
    // 更新各类信息
    void updateNetDevice(const QString &deviceUni);
    void updateWiredConnection(const QString &deviceUni, const QString &connectionUuid);
    void updateWirelessConnection(const QString &deviceUni, const QString &ssid);

    // 有线网络接入点变化
    void wiredNetworkAppeared(const QString &deviceUni, const QString &connectionUuid);
    void wiredNetworkDisappeared(const QString &deviceUni, const QString &connectionUuid);

    // 无线网络接入点变化
    void wirelessNetworkAppeared(const QString &deviceUni, const QString &ssid);
    void wirelessNetworkDisappeared(const QString &deviceUni, const QString &ssid);

    void removeConnection(const QString &deviceUni, const QString &connectUuid);

    // 连接状态变化
    void updateActiveStatus(const QString &deviceUni, NetworkManager::Device::State state);
    void activeConnectionStateChanged(const QString &deviceUni, NetworkManager::ActiveConnection::State state);

    void requestPassword(const QString &devicePath, const QString &ssid, bool wait);

private:
    QMap<QString, QPair<QTreeWidgetItem *, QWidget *>> m_deviceItems;                     // <device uuid,<>>
    QMap<QString, QMap<QString, QPair<QTreeWidgetItem *, QWidget *>>> m_connectionItems;  // <device uuid, <connectuuid/ssid, <>>>

    NetworkManager::Device::Type m_netType;
};
}  // namespace SettingBar
}  // namespace Kiran
