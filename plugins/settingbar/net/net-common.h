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

#include <NetworkManagerQt/ActiveConnection>
#include <NetworkManagerQt/ConnectionSettings>
#include <NetworkManagerQt/Device>
#include <NetworkManagerQt/Manager>

// 网络公共接口

namespace Kiran
{
namespace SettingBar
{
enum OpeartionType
{
    OPERTION_ACTIVATE,
    OPERTION_DEACTIVATE
};

enum OpeartionResult
{
    OPERTION_SUCCEEDED,
    OPERTION_FAILED
};

#define NetCommonInstance NetCommon::getInstance()

class NetCommon : public QObject
{
    Q_OBJECT
public:
    static NetCommon& getInstance();

    static bool hasEthernetDevices();
    static bool hasWifiDevices();

    // 获取网卡设备
    static NetworkManager::Device::List getEthernetDevices();
    static NetworkManager::Device::List getWifiDevices();
    static NetworkManager::Device::List getDevices(const NetworkManager::Device::Type& type);

    // 连接网络设备
    static bool disconnectEthernet();
    static bool disconnectWifi();
    static bool disconnectDevice(const NetworkManager::Device::Type& type);
    static bool disconnectDevice(const QString& deviceUni);
    static bool activateConnection(const QString& deviceUni, const QString& connectionUuid);
    static bool deactivateConnection(const QString& connectionUuid);

    // wifi
    static NetworkManager::Connection::Ptr getAvailableConnectionBySsid(const QString& deviceUni, const QString& ssid);

    // 获取当前网络图标 key:图标 value:tooltip
    static QPair<QString, QString> getNetworkIcon();

    // 连接结果
    void checkOpeartionResult(OpeartionType type, QString name, QDBusPendingCall& call);

private:
    NetCommon();

    enum NetworkState
    {
        UNKNOWN,
        WIRED_CONNECTED,
        WIRED_CONNECTED_BUT_NOT_ACCESS_INTERNET,
        WIRELESS_CONNECTED,
        WIRELESS_CONNECTED_BUT_NOT_ACCESS_INTERNET,
        DISCONNECTED
    };

    static QPair<QString, QString> getNetworkIcon(const NetworkState& state);
    static QPair<QString, QString> getNetworkIcon(const NetworkManager::ActiveConnection::Ptr& connection);
    static NetworkManager::Connectivity checkConnectivity();

    // 连接结果
    void processPendingCallFinished(QDBusPendingCallWatcher* watcher);

signals:
    void netStatusChanged();
};
}  // namespace SettingBar
}  // namespace Kiran
