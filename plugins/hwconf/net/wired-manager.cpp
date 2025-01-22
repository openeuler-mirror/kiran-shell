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

#include <qt5-log-i.h>
#include <NetworkManagerQt/Device>
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/Settings>
#include <NetworkManagerQt/WiredDevice>

#include "wired-manager.h"

namespace Kiran
{
namespace HwConf
{
WiredManager::WiredManager(QObject *parent)
    : QObject{parent}
{
}

WiredManager &WiredManager::getInstance()
{
    static WiredManager instance;
    return instance;
}

void WiredManager::AddToManager(const QString &deviceUni)
{
    if (!m_deviceUnis.contains(deviceUni))
    {
        m_deviceUnis.append(deviceUni);
        auto device = NetworkManager::findNetworkInterface(deviceUni);
        NetworkManager::WiredDevice::Ptr wiredDevice = device.objectCast<NetworkManager::WiredDevice>();

        connect(device.data(), &NetworkManager::Device::activeConnectionChanged, this, &WiredManager::activeConnectionChanged);
        connect(device.data(), &NetworkManager::Device::availableConnectionAppeared, [this, deviceUni](const QString &connectionPath)
                {
                    auto connection = NetworkManager::findConnection(connectionPath);
                    emit availableConnectionAppeared(deviceUni, connection->uuid());
                });

        connect(device.data(), &NetworkManager::Device::availableConnectionDisappeared, [this, deviceUni](const QString &connectionPath)
                {
                    auto connection = NetworkManager::findConnection(connectionPath);
                    emit availableConnectionDisappeared(deviceUni, connection->uuid());
                });
        // 这个信号捕获不到 连接已active，设备active了，连接仍未active，需要NetworkManager::Device::activeConnectionChanged
        connect(device.data(), &NetworkManager::Device::stateChanged, [this, deviceUni](NetworkManager::Device::State newstate, NetworkManager::Device::State oldstate, NetworkManager::Device::StateChangeReason reason)
                {
                    KLOG_INFO() << "WirelessNetworkManager::stateChanged" << newstate;
                    emit stateChanged(deviceUni, newstate);
                });  // this, &WirelessManager::stateChanged);
    }
}

void WiredManager::RemoveFromManager(const QString &deviceUni)
{
    if (m_deviceUnis.contains(deviceUni))
    {
        m_deviceUnis.removeAll(deviceUni);
    }
}

}  // namespace HwConf
}  // namespace Kiran
