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

#include <NetworkManagerQt/Device>
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/WirelessDevice>

#include "wireless-manager.h"

namespace Kiran
{
namespace HwConf
{
WirelessManager::WirelessManager(QObject *parent)
    : QObject{parent}
{
}

WirelessManager &WirelessManager::getInstance()
{
    static WirelessManager instance;
    return instance;
}

void WirelessManager::AddToManager(const QString &deviceUni)
{
    if (!m_deviceManagerMap.contains(deviceUni))
    {
        m_deviceManagerMap[deviceUni] = new WirelessNetworkManager(deviceUni);
        auto device = NetworkManager::findNetworkInterface(deviceUni);
        NetworkManager::WirelessDevice::Ptr wirelessDevice = device.objectCast<NetworkManager::WirelessDevice>();

        connect(m_deviceManagerMap[deviceUni], &WirelessNetworkManager::networkAppeared, [this, deviceUni](QString ssid)
                {
                    emit networkAppeared(deviceUni, ssid);
                });
        connect(m_deviceManagerMap[deviceUni], &WirelessNetworkManager::networkDisappeared, [this, deviceUni](QString ssid)
                {
                    emit networkDisappeared(deviceUni, ssid);
                });

        for (auto networkInfo : m_deviceManagerMap[deviceUni]->getNetworkInfoList())
        {
            emit networkAppeared(deviceUni, networkInfo.ssid);
        }
    }
}

void WirelessManager::RemoveFromManager(const QString &deviceUni)
{
    if (m_deviceManagerMap.contains(deviceUni))
    {
        auto deviceManager = m_deviceManagerMap[deviceUni];
        delete deviceManager;
        m_deviceManagerMap.remove(deviceUni);
    }
}

WirelessNetworkInfoList WirelessManager::getNetworkInfoList(const QString &deviceUni)
{
    if (!m_deviceManagerMap.contains(deviceUni))
    {
        return WirelessNetworkInfoList();
    }

    return m_deviceManagerMap[deviceUni]->getNetworkInfoList();
}

void WirelessManager::addAndActivateConnection(const QString &deviceUni, const QString &ssid, const QString &password)
{
    if (!m_deviceManagerMap.contains(deviceUni))
    {
        return;
    }
    m_deviceManagerMap[deviceUni]->addAndActivateNetwork(ssid, password);
}
}  // namespace HwConf
}  // namespace Kiran
