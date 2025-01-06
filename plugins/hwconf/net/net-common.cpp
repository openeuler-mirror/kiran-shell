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

#include <networkmanagerqt/wirelesssetting.h>
#include <qt5-log-i.h>
#include <QDBusReply>

#include "ks-i.h"
#include "net-common.h"

static void DumpDeviceInfo(NetworkManager::Device::List devices)
{
    for (const auto &device : devices)
    {
        KLOG_INFO() << "设备名称:" << device->interfaceName();
        KLOG_INFO() << "设备类型:" << device->type();
        KLOG_INFO() << "设备状态:" << device->state();

        // 获取设备的连接
        const auto availableConnections = device->availableConnections();
        for (const auto &availableConnection : availableConnections)
        {
            KLOG_INFO() << "  连接 UUID:" << availableConnection->uuid();
            KLOG_INFO() << "  path:" << availableConnection->path();
            KLOG_INFO() << "  name:" << availableConnection->name();
        }

        // 获取设备 IP 地址
        const auto ipv4Config = device->ipV4Config();
        if (ipv4Config.isValid())
        {
            KLOG_INFO() << "  IPv4 信息:\n"
                        << "    domains:" << ipv4Config.domains()
                        << "    gateway:" << ipv4Config.gateway()
                        << "    nameservers:" << ipv4Config.nameservers()
                        << "    dnsOptions:" << ipv4Config.dnsOptions();
            KLOG_INFO() << "  IPv4 地址:";
            for (auto addr : ipv4Config.addresses())
            {
                KLOG_INFO() << "    " << addr.ip() << addr.gateway();
            }
        }

        const auto ipv6Config = device->ipV6Config();
        if (ipv6Config.isValid())
        {
            KLOG_INFO() << "  IPv6 信息:"
                        << "    domains:" << ipv6Config.domains()
                        << "    gateway:" << ipv6Config.gateway()
                        << "    nameservers:" << ipv6Config.nameservers()
                        << "    dnsOptions:" << ipv6Config.dnsOptions();
            KLOG_INFO() << "  IPv6 地址:";
            for (auto addr : ipv6Config.addresses())
            {
                KLOG_INFO() << "    " << addr.ip() << addr.gateway();
            }
        }
    }
}

static void TestDevice()
{
    // 获取所有网络设备
    const auto devices = NetworkManager::networkInterfaces();
    KLOG_INFO() << "发现的网络设备数量:" << devices.size();
    // 打印 NetworkManager 管理器级别的信息
    KLOG_INFO() << "NetworkManager 状态:" << NetworkManager::status();

    DumpDeviceInfo(devices);
}

namespace Kiran
{
namespace HwConf
{
NetCommon::NetCommon()
{
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::statusChanged, this, &NetCommon::netStatusChanged);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::deviceAdded, this, &NetCommon::netStatusChanged);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::deviceRemoved, this, &NetCommon::netStatusChanged);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::wirelessEnabledChanged, this, &NetCommon::netStatusChanged);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::wwanEnabledChanged, this, &NetCommon::netStatusChanged);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::wimaxEnabledChanged, this, &NetCommon::netStatusChanged);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::wirelessHardwareEnabledChanged, this, &NetCommon::netStatusChanged);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::wwanHardwareEnabledChanged, this, &NetCommon::netStatusChanged);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::wimaxHardwareEnabledChanged, this, &NetCommon::netStatusChanged);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::networkingEnabledChanged, this, &NetCommon::netStatusChanged);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::serviceDisappeared, this, &NetCommon::netStatusChanged);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::serviceAppeared, this, &NetCommon::netStatusChanged);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::connectivityChanged, this, &NetCommon::netStatusChanged);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::primaryConnectionChanged, this, &NetCommon::netStatusChanged);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::activatingConnectionChanged, this, &NetCommon::netStatusChanged);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::primaryConnectionTypeChanged, this, &NetCommon::netStatusChanged);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::meteredChanged, this, &NetCommon::netStatusChanged);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::globalDnsConfigurationChanged, this, &NetCommon::netStatusChanged);
}

NetCommon &NetCommon::getInstance()
{
    static NetCommon instance;
    return instance;
}

bool NetCommon::hasEthernetDevices()
{
    return !NetCommon::getEthernetDevices().isEmpty();
}

bool NetCommon::hasWifiDevices()
{
    return !NetCommon::getWifiDevices().isEmpty();
}

NetworkManager::Device::List NetCommon::getDevices(const NetworkManager::Device::Type &type)
{
    auto devices = NetworkManager::Device::List();
    for (const auto &device : NetworkManager::networkInterfaces())
    {
        if (device->type() == type)
        {
            devices.append(device);
        }
    }

    //    DumpDeviceInfo(devices);

    return devices;
}

NetworkManager::Device::List NetCommon::getEthernetDevices()
{
    return getDevices(NetworkManager::Device::Ethernet);
}

NetworkManager::Device::List NetCommon::getWifiDevices()
{
    return getDevices(NetworkManager::Device::Wifi);
}

bool NetCommon::disconnectEthernet()
{
    return disconnectDevice(NetworkManager::Device::Ethernet);
}

bool NetCommon::disconnectWifi()
{
    return disconnectDevice(NetworkManager::Device::Wifi);
}

bool NetCommon::disconnectDevice(const NetworkManager::Device::Type &type)
{
    NetworkManager::Device::List devices;
    if (NetworkManager::Device::Ethernet == type)
    {
        devices = NetCommon::getEthernetDevices();
    }
    else if (NetworkManager::Device::Wifi == type)
    {
        devices = NetCommon::getWifiDevices();
    }

    for (auto device : devices)
    {
        QDBusReply<void> reply = device->disconnectInterface();
        if (!reply.isValid())
        {
            KLOG_ERROR() << "断连设备失败:"
                         << "name:" << device->interfaceName()
                         << "uuid:" << device->uni()
                         << "error:" << reply.error().message();
            return false;
        }
        else
        {
            KLOG_INFO() << "断连设备成功:"
                        << "name:" << device->interfaceName()
                        << "uuid:" << device->uni();
            return true;
        }
    }
    return true;
}

bool NetCommon::disconnectDevice(const QString &deviceUni)
{
    for (const auto &device : NetworkManager::networkInterfaces())
    {
        if (device->uni() == deviceUni)
        {
            QDBusReply<void> reply = device->disconnectInterface();
            if (!reply.isValid())
            {
                KLOG_ERROR() << "断连设备失败:"
                             << "name:" << device->interfaceName()
                             << "uuid:" << device->uni()
                             << "error:" << reply.error().message();
                return false;
            }
            else
            {
                KLOG_INFO() << "断连设备成功:"
                            << "name:" << device->interfaceName()
                            << "uuid:" << device->uni();
                return true;
            }
        }
    }

    return true;
}

bool NetCommon::activateConnection(const QString &deviceUni, const QString &connectionUuid)
{
    for (auto device : NetworkManager::networkInterfaces())
    {
        for (const auto &availableConnection : device->availableConnections())
        {
            if (availableConnection->uuid() == connectionUuid)
            {
                // 连接网络
                QDBusReply<QDBusObjectPath> reply = NetworkManager::activateConnection(availableConnection->path(), deviceUni, QString());
                if (!reply.isValid())
                {
                    KLOG_ERROR() << "activate connection failed:"
                                 << "device uni:" << deviceUni
                                 << "connection uuid:" << connectionUuid
                                 << "connection path:" << availableConnection->path()
                                 << "error:" << reply.error().message();
                    return false;
                }
                else
                {
                    KLOG_INFO() << "activate connection success:"
                                << "device uni:" << deviceUni
                                << "connection uuid:" << connectionUuid;
                    return true;
                }
            }
        }
    }

    KLOG_ERROR() << "find connection failed when activate connection:"
                 << "device uni:" << deviceUni
                 << "connection uuid:" << connectionUuid;
    return false;
}

bool NetCommon::deactivateConnection(const QString &connectionUuid)
{
    for (const NetworkManager::ActiveConnection::Ptr &active : NetworkManager::activeConnections())
    {
        if (active->uuid() == connectionUuid)
        {
            QDBusReply<void> reply = NetworkManager::deactivateConnection(active->path());
            if (!reply.isValid())
            {
                KLOG_ERROR() << "deactivate connection failed:"
                             << "connection uuid:" << connectionUuid
                             << "active connection path:" << active->path()
                             << "error:" << reply.error().message();
                return false;
            }
            else
            {
                KLOG_INFO() << "deactivate connection success:"
                            << "connection uuid:" << connectionUuid;

                return true;
            }

            break;
        }
    }

    KLOG_ERROR() << "find connection failed when deactivate connection:"
                 << "connection uuid:" << connectionUuid;
    return false;
}

NetworkManager::Connection::Ptr NetCommon::getAvailableConnectionBySsid(const QString &deviceUni, const QString &ssid)
{
    auto device = NetworkManager::findNetworkInterface(deviceUni);
    auto availableConnections = device->availableConnections();
    for (auto connection : availableConnections)
    {
        if (connection->settings()->connectionType() == NetworkManager::ConnectionSettings::Wireless)
        {
            auto wirelessSetting = connection->settings()->setting(NetworkManager::Setting::SettingType::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
            if (ssid == QString(wirelessSetting->ssid()))
            {
                return connection;
            }
        }
    }

    return NetworkManager::Connection::Ptr();
}

QPair<QString, QString> NetCommon::getNetworkIcon()
{
    auto status = NetworkManager::status();
    if (status < NetworkManager::Status::ConnectedLinkLocal)
    {
        return getNetworkIcon(DISCONNECTED);
    }

    // the "primary" active connection being used to access the network
    auto primaryConnection = NetworkManager::primaryConnection();
    if (!primaryConnection.isNull())
    {
        return getNetworkIcon(primaryConnection);
    }

    // 网络已连接，但主连接为空
    auto activeConnections = NetworkManager::activeConnections();
    NetworkManager::ActiveConnection::Ptr topLevelConnection;
    // 优先显示有线连接图标
    for (auto activeConnection : activeConnections)
    {
        auto type = activeConnection->type();
        auto state = activeConnection->state();
        if (NetworkManager::ActiveConnection::Activated != state)
        {
            continue;
        }
        switch (type)
        {
        case NetworkManager::ConnectionSettings::Wired:
        {
            topLevelConnection = activeConnection;
            break;
        }
        case NetworkManager::ConnectionSettings::Wireless:
        {
            if (topLevelConnection.isNull())
            {
                topLevelConnection = activeConnection;
            }
            else if (!topLevelConnection.isNull() && topLevelConnection->type() != NetworkManager::ConnectionSettings::Wireless)
            {
                topLevelConnection = activeConnection;
            }

            break;
        }
        // TODO:其他设备，如果有需求，再完善
        case NetworkManager::ConnectionSettings::Adsl:
        case NetworkManager::ConnectionSettings::Bluetooth:
        case NetworkManager::ConnectionSettings::Bond:
        case NetworkManager::ConnectionSettings::Bridge:
        case NetworkManager::ConnectionSettings::Cdma:
        case NetworkManager::ConnectionSettings::Gsm:
        case NetworkManager::ConnectionSettings::Infiniband:
        case NetworkManager::ConnectionSettings::OLPCMesh:
        case NetworkManager::ConnectionSettings::Pppoe:
        case NetworkManager::ConnectionSettings::Vlan:
        case NetworkManager::ConnectionSettings::Vpn:
        case NetworkManager::ConnectionSettings::Wimax:
        case NetworkManager::ConnectionSettings::Team:
        case NetworkManager::ConnectionSettings::Generic:
        case NetworkManager::ConnectionSettings::Tun:
        case NetworkManager::ConnectionSettings::IpTunnel:
        case NetworkManager::ConnectionSettings::WireGuard:
        default:
        {
            break;
        }
        }

        if (!topLevelConnection.isNull() && topLevelConnection->type() == NetworkManager::ConnectionSettings::Wired)
        {
            break;
        }
    }
    if (!topLevelConnection.isNull())
    {
        return getNetworkIcon(topLevelConnection);
    }
    else
    {
        return getNetworkIcon(DISCONNECTED);
    }
}

QPair<QString, QString> NetCommon::getNetworkIcon(const NetworkState &state)
{
    QMap<NetworkState, QPair<QString, QString>> statusWithIcon = {
        {UNKNOWN, {KS_ICON_WIRED_ERROR, tr("Network unknow error")}},
        {WIRED_CONNECTED, {KS_ICON_WIRED_CONNECTED, tr("Network connected")}},
        {WIRED_CONNECTED_BUT_NOT_ACCESS_INTERNET, {KS_ICON_WIRED_ERROR, tr("The network is connected, but you cannot access the Internet")}},
        {WIRELESS_CONNECTED, {KS_ICON_WIRELESS_3, tr("Network connected")}},
        {WIRELESS_CONNECTED_BUT_NOT_ACCESS_INTERNET, {KS_ICON_WIRED_ERROR, tr("The network is connected, but you cannot access the Internet")}},
        {DISCONNECTED, {KS_ICON_NET_DISCONNECTED, tr("Network disconnected")}}};

    return statusWithIcon[state];
}

QPair<QString, QString> NetCommon::getNetworkIcon(const NetworkManager::ActiveConnection::Ptr &connection)
{
    NetworkManager::Connectivity connectivity = checkConnectivity();

    switch (connection->type())
    {
    case NetworkManager::ConnectionSettings::Wired:
    {
        if (connectivity == NetworkManager::Connectivity::Full)
        {
            return getNetworkIcon(WIRED_CONNECTED);
        }
        else
        {
            return getNetworkIcon(WIRED_CONNECTED_BUT_NOT_ACCESS_INTERNET);
        }
    }
    case NetworkManager::ConnectionSettings::Wireless:
    {
        if (connectivity == NetworkManager::Connectivity::Full)
        {
            return getNetworkIcon(WIRELESS_CONNECTED);
        }
        else
        {
            return getNetworkIcon(WIRED_CONNECTED_BUT_NOT_ACCESS_INTERNET);
        }
    }
    default:
    {
        return getNetworkIcon(UNKNOWN);
    }
    }
}

NetworkManager::Connectivity NetCommon::checkConnectivity()
{
    QDBusPendingReply<uint> reply = NetworkManager::checkConnectivity();
    reply.waitForFinished();
    if (reply.isError())
    {
        return NetworkManager::Connectivity::UnknownConnectivity;
    }
    return (NetworkManager::Connectivity)reply.value();
}

}  // namespace HwConf
}  // namespace Kiran
