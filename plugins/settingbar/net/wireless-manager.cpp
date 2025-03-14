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

#include "lib/common/logging-category.h"
#include "net-common.h"
#include "wireless-manager.h"

namespace Kiran
{
namespace SettingBar
{
WirelessManager::WirelessManager(QObject *parent)
    : QObject{parent}
{
    // 尝试连接时，SecretAgent请求密码，弹出密码框
    m_secretAgent = new NMSecretAgent(this);
    connect(m_secretAgent, &NMSecretAgent::requestPassword, this, &WirelessManager::requestPassword);

    connect(&NetCommonInstance, &NetCommon::netStatusChanged, this, &WirelessManager::updateNetworkStatus);
    updateNetworkStatus();
}

void WirelessManager::changeActiveConnection()
{
    auto *device = qobject_cast<NetworkManager::Device *>(sender());
    auto deviceUni = device->uni();

    auto activeConnection = device->activeConnection();
    if (activeConnection)
    {
        connect(activeConnection.data(), &NetworkManager::ActiveConnection::stateChanged, [this, deviceUni](NetworkManager::ActiveConnection::State state)
                {
                    emit activeConnectionStateChanged(deviceUni, state);
                });
    }
}

void WirelessManager::updateNetworkStatus()
{
    QStringList currentDeviceUnis;
    NetworkManager::Device::List devices = NetCommonInstance.getWifiDevices();
    for (const auto &device : devices)
    {
        currentDeviceUnis.append(device->uni());
    }

    for (const auto &uni : currentDeviceUnis)
    {
        if (m_deviceManagerMap.contains(uni))
        {
            continue;
        }
        AddToManager(uni);
    }

    for (const auto &uni : m_deviceManagerMap.keys())
    {
        if (currentDeviceUnis.contains(uni))
        {
            continue;
        }
        RemoveFromManager(uni);
    }

    emit netStatusChanged();
}

WirelessManager &WirelessManager::getInstance()
{
    static WirelessManager instance;
    return instance;
}

QStringList WirelessManager::getDevices()
{
    return m_deviceManagerMap.keys();
}

void WirelessManager::AddToManager(const QString &deviceUni)
{
    if (!m_deviceManagerMap.contains(deviceUni))
    {
        auto device = NetworkManager::findNetworkInterface(deviceUni);
        auto wirelessDevice = device.objectCast<NetworkManager::WirelessDevice>();
        m_deviceManagerMap[deviceUni] = new WirelessNetworkManager(wirelessDevice);

        // 连接过程状态
        connect(device.data(), &NetworkManager::Device::activeConnectionChanged, this, &WirelessManager::changeActiveConnection);
        // 连接点增加、减少
        connect(m_deviceManagerMap[deviceUni], &WirelessNetworkManager::networkAppeared, [this, deviceUni](const QString &ssid)
                {
                    emit networkAppeared(deviceUni, ssid);
                });
        connect(m_deviceManagerMap[deviceUni], &WirelessNetworkManager::networkDisappeared, [this, deviceUni](const QString &ssid)
                {
                    emit networkDisappeared(deviceUni, ssid);
                });

        //        connect(device.data(), &NetworkManager::Device::stateChanged, [this, deviceUni](NetworkManager::Device::State newstate, NetworkManager::Device::State oldstate, NetworkManager::Device::StateChangeReason reason)
        //                {
        //                    KLOG_INFO(LCSettingbar) << "WirelessNetworkManager::stateChanged" << newstate << oldstate << reason;
        //                    emit stateChanged(deviceUni, newstate);
        //                });

        for (const auto &networkInfo : m_deviceManagerMap[deviceUni]->getNetworkInfoList())
        {
            emit networkAppeared(deviceUni, networkInfo.ssid);
        }
    }
}

void WirelessManager::RemoveFromManager(const QString &deviceUni)
{
    if (m_deviceManagerMap.contains(deviceUni))
    {
        auto *deviceManager = m_deviceManagerMap[deviceUni];
        delete deviceManager;
        m_deviceManagerMap.remove(deviceUni);
    }
}

WirelessNetworkInfoList WirelessManager::getNetworkInfoList(const QString &deviceUni)
{
    if (!m_deviceManagerMap.contains(deviceUni))
    {
        return {};
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
    KLOG_INFO(LCSettingbar) << "wireless activate network" << deviceUni << ssid;

    auto pendingReply = m_deviceManagerMap[deviceUni]->activateNetowrk(ssid);
    NetCommonInstance.checkOpeartionResult(OPERTION_ACTIVATE, ssid, pendingReply);
}

void WirelessManager::addAndActivateNetwork(const QString &deviceUni, const QString &ssid, const QString &password)
{
    if (!m_deviceManagerMap.contains(deviceUni))
    {
        return;
    }

    KLOG_INFO(LCSettingbar) << "wireless add and activate network" << deviceUni << ssid;

    auto pendingReply = m_deviceManagerMap[deviceUni]->addAndActivateNetwork(ssid, password);
    NetCommonInstance.checkOpeartionResult(OPERTION_ACTIVATE, ssid, pendingReply);
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

    KLOG_INFO(LCSettingbar) << "wireless scan" << deviceUni;

    m_deviceManagerMap[deviceUni]->requestScan();
}
}  // namespace SettingBar
}  // namespace Kiran
