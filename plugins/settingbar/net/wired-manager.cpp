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

#include "lib/common/logging-category.h"
#include "net-common.h"
#include "wired-manager.h"

namespace Kiran
{
namespace SettingBar
{
WiredManager::WiredManager(QObject *parent)
    : QObject{parent}
{
    connect(&NetCommonInstance, &NetCommon::netStatusChanged, this, &WiredManager::updateNetworkStatus);
    updateNetworkStatus();
}

void WiredManager::updateNetworkStatus()
{
    QStringList currentDeviceUnis;
    NetworkManager::Device::List devices = NetCommonInstance.getEthernetDevices();
    for (const auto &device : devices)
    {
        currentDeviceUnis.append(device->uni());
    }

    m_deviceUnis.erase(std::remove_if(m_deviceUnis.begin(), m_deviceUnis.end(), [&currentDeviceUnis](const QString &uni)
                                      {
                                          return !currentDeviceUnis.contains(uni);
                                      }),
                       m_deviceUnis.end());

    for (const auto &uni : currentDeviceUnis)
    {
        if (m_deviceUnis.contains(uni))
        {
            continue;
        }
        AddToManager(uni);
    }

    emit netStatusChanged();
}

WiredManager &WiredManager::getInstance()
{
    static WiredManager instance;
    return instance;
}

QStringList WiredManager::getDevices()
{
    return m_deviceUnis;
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
                    KLOG_INFO(LCSettingbar) << "WirelessNetworkManager::stateChanged" << newstate;
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

}  // namespace SettingBar
}  // namespace Kiran
