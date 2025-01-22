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

#include <kiran-desktop/nm-secret-agent.h>
#include <kiran-desktop/wireless-network-manager.h>
#include <qt5-log-i.h>
#include <NetworkManagerQt/Device>
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/WirelessDevice>
#include <QInputDialog>
#include <thread>

#include "wireless-manager.h"

namespace Kiran
{
namespace HwConf
{
WirelessManager::WirelessManager(QObject *parent)
    : QObject{parent}
{
    // 尝试连接时，SecretAgent请求密码，弹出密码框
    m_secretAgent = new NMSecretAgent(this);
    connect(m_secretAgent, &NMSecretAgent::requestPassword, this, &WirelessManager::requestPassword);
}

void WirelessManager::changeActiveConnection()
{
    auto device = qobject_cast<NetworkManager::Device *>(sender());
    auto deviceUni = device->uni();

    auto activeConnection = device->activeConnection();
    if (activeConnection)
    {
        if (!m_deviceActiveConnectMap.contains(deviceUni))
        {
            m_deviceActiveConnectMap[deviceUni] = activeConnection;
            connect(activeConnection.data(), &NetworkManager::ActiveConnection::stateChanged, [this, deviceUni](NetworkManager::ActiveConnection::State state)
                    {
                        emit activeConnectionStateChanged(deviceUni, state);
                    });
        }
    }
    else
    {
        if (m_deviceActiveConnectMap.contains(deviceUni))
        {
            m_deviceActiveConnectMap.remove(deviceUni);
        }
    }
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
        auto device = NetworkManager::findNetworkInterface(deviceUni);
        auto wirelessDevice = device.objectCast<NetworkManager::WirelessDevice>();
        m_deviceManagerMap[deviceUni] = new WirelessNetworkManager(wirelessDevice);

        connect(device.data(), &NetworkManager::Device::activeConnectionChanged, this, &WirelessManager::changeActiveConnection);

        connect(m_deviceManagerMap[deviceUni], &WirelessNetworkManager::networkAppeared, [this, deviceUni](const QString &ssid)
                {
                    emit networkAppeared(deviceUni, ssid);
                });
        connect(m_deviceManagerMap[deviceUni], &WirelessNetworkManager::networkDisappeared, [this, deviceUni](const QString &ssid)
                {
                    emit networkDisappeared(deviceUni, ssid);
                });

        connect(device.data(), &NetworkManager::Device::stateChanged, [this, deviceUni](NetworkManager::Device::State newstate, NetworkManager::Device::State oldstate, NetworkManager::Device::StateChangeReason reason)
                {
                    KLOG_INFO() << "WirelessNetworkManager::stateChanged" << newstate << oldstate << reason;
                    emit stateChanged(deviceUni, newstate);
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

WifiSecurityType WirelessManager::networkBestSecurityType(const QString &deviceUni, const QString &ssid)
{
    return m_deviceManagerMap[deviceUni]->networkBestSecurityType(ssid);
}

bool WirelessManager::checkNetworkCanDirectConn(const QString &deviceUni, const QString &ssid)
{
    return m_deviceManagerMap[deviceUni]->checkNetworkCanDirectConn(ssid);
}

void WirelessManager::activateNetowrk(const QString &deviceUni, const QString &ssid)
{
    return m_deviceManagerMap[deviceUni]->activateNetowrk(ssid);
}

void WirelessManager::addAndActivateNetwork(const QString &deviceUni, const QString &ssid, const QString &password)
{
    KLOG_INFO() << "WirelessManager::addAndActivateNetwork" << deviceUni << ssid;
    if (!m_deviceManagerMap.contains(deviceUni))
    {
        return;
    }
    m_deviceManagerMap[deviceUni]->addAndActivateNetwork(ssid, password);
}

void WirelessManager::respondPasswdRequest(const QString &ssid, const QString &password, bool isCancel)
{
    m_secretAgent->respondPasswdRequest(ssid, password, isCancel);
}
void WirelessManager::scan(const QString &deviceUni)
{
    if (!m_deviceManagerMap.contains(deviceUni))
    {
        return;
    }
    m_deviceManagerMap[deviceUni]->requestScan();
}
}  // namespace HwConf
}  // namespace Kiran
