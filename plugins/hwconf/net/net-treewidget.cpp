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
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/Settings>
#include <QHeaderView>

#include "device-widget.h"
#include "net-common.h"
#include "net-treewidget.h"
#include "wired-connection-widget.h"
#include "wired-manager.h"
#include "wireless-connection-widget.h"
#include "wireless-manager.h"

namespace Kiran
{
namespace HwConf
{
netTreeWidget::netTreeWidget(NetworkManager::Device::Type deviceType, QWidget *parent)
    : QTreeWidget(parent),
      m_netType(deviceType)
{
    if (NetworkManager::Device::Type::Ethernet == m_netType)
    {
        // 有线接入点变化信号
        connect(&WiredManagerInstance, &WiredManager::availableConnectionAppeared, this, &netTreeWidget::wiredNetworkAppeared);
        connect(&WiredManagerInstance, &WiredManager::availableConnectionDisappeared, this, &netTreeWidget::wiredNetworkDisappeared);
    }
    else if (NetworkManager::Device::Type::Wifi == m_netType)
    {
        // 无线接入点变化信号
        connect(&WirelessManagerInstance, &WirelessManager::networkAppeared, this, &netTreeWidget::wirelessNetworkAppeared);
        connect(&WirelessManagerInstance, &WirelessManager::networkDisappeared, this, &netTreeWidget::wirelessNetworkDisappeared);
    }

    // 激活连接信号
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::activeConnectionsChanged, this, &netTreeWidget::activeConnectionChanged);

    // 其他信号
    //    connect(&NetworkInstance, &Network::netStatusChanged, this, &netTreeWidget::updateNetworkStatus);

    updateNetworkStatus();
    expandAll();

    // 表头隐藏
    setHeaderHidden(true);
    // 设置 QTreeWidget 点击无颜色变化
    setSelectionMode(QAbstractItemView::NoSelection);
    setFocusPolicy(Qt::NoFocus);

    // 背景透明
    QPalette p = this->palette();
    p.setBrush(QPalette::Base, QBrush(QColor(0, 0, 0, 0)));
    setPalette(p);
}

void netTreeWidget::updateNetworkStatus()
{
    KLOG_INFO() << "ConnectTreeWidget::updateNetworkStatus";

    NetworkManager::Device::List devices;
    if (NetworkManager::Device::Type::Ethernet == m_netType)
    {
        devices = NetCommonInstance.getEthernetDevices();
    }
    else if (NetworkManager::Device::Type::Wifi == m_netType)
    {
        devices = NetCommonInstance.getWifiDevices();
    }

    QStringList deviceUnis;
    for (const auto &device : devices)
    {
        // 添加网卡设备
        updateNetDevice(device);
        deviceUnis.append(device->uni());

        if (NetworkManager::Device::Type::Ethernet == m_netType)
        {
            // 有线管理
            WiredManagerInstance.AddToManager(device->uni());
        }
        else if (NetworkManager::Device::Type::Wifi == m_netType)
        {
            // 无线管理
            WirelessManagerInstance.AddToManager(device->uni());
        }
    }

    // 移除不存在的网卡
    for (auto deviceUni : m_netDeviceItems.keys())
    {
        if (!deviceUnis.contains(deviceUni))
        {
            QTreeWidgetItem *item = m_netDeviceItems[deviceUni].first;
            QWidget *widget = m_netDeviceItems[deviceUni].second;
            takeTopLevelItem(indexOfTopLevelItem(item));

            delete item;
            delete widget;

            m_netDeviceItems.remove(deviceUni);

            if (NetworkManager::Device::Type::Ethernet == m_netType)
            {
                WiredManagerInstance.RemoveFromManager(deviceUni);
            }
            else if (NetworkManager::Device::Type::Wifi == m_netType)
            {
                WirelessManagerInstance.RemoveFromManager(deviceUni);
            }
        }
    }
}

void netTreeWidget::updateNetDevice(NetworkManager::Device::Ptr device)
{
    qInfo() << "netTreeWidget::updateNetDevice" << device->uni();

    QString deviceUni = device->uni();
    if (!m_netDeviceItems.contains(deviceUni))
    {
        auto netDeviceItem = new DeviceWidget(m_netType, deviceUni, this);
        QTreeWidgetItem *treeWidgetItem = new QTreeWidgetItem(this);
        setItemWidget(treeWidgetItem, 0, netDeviceItem);

        m_netDeviceItems[deviceUni] = qMakePair(treeWidgetItem, netDeviceItem);

        netDeviceItem->Init();
    }

    QString connecttionUuids;
    if (NetworkManager::Device::Type::Ethernet == m_netType)
    {
        // 添加有线连接
        for (const auto &connection : device->availableConnections())
        {
            QString connectionUuid = connection->uuid();
            updateWiredConnection(deviceUni, connectionUuid);

            connecttionUuids.append(connection->uuid());
        }
    }
    else if (NetworkManager::Device::Type::Wifi == m_netType)
    {
        // 添加无线连接
        auto networkInfos = WirelessManagerInstance.getNetworkInfoList(deviceUni);
        for (auto networkInfo : networkInfos)
        {
            updateWirelessConnection(deviceUni, networkInfo.ssid);
            connecttionUuids.append(networkInfo.ssid);
        }
    }

    // 移除不存在的连接
    auto connectionMap = m_netConnectionItem[device->uni()];
    for (QString connectUuid : connectionMap.keys())
    {
        if (!connecttionUuids.contains(connectUuid))
        {
            removeConnection(device->uni(), connectUuid);
        }
    }
}

void netTreeWidget::updateWiredConnection(QString deviceUni, QString connectionUuid)
{
    qInfo() << "netTreeWidget::updateNetConnection" << deviceUni << connectionUuid;

    WiredConnectionWidget *netConnectionItem;
    if (!m_netConnectionItem[deviceUni].contains(connectionUuid))
    {
        netConnectionItem = new WiredConnectionWidget(deviceUni, connectionUuid, this);
        QTreeWidgetItem *treeWidgetItem = new QTreeWidgetItem(m_netDeviceItems[deviceUni].first);
        setItemWidget(treeWidgetItem, 0, netConnectionItem);

        m_netConnectionItem[deviceUni][connectionUuid] = qMakePair(treeWidgetItem, netConnectionItem);
    }
    else
    {
        netConnectionItem = (WiredConnectionWidget *)m_netConnectionItem[deviceUni][connectionUuid].second;
    }

    netConnectionItem->updateStatus();
}

void netTreeWidget::updateWirelessConnection(QString deviceUni, QString ssid)
{
    qInfo() << "netTreeWidget::updateNetConnectionWifi" << deviceUni << ssid;

    // 信号强度变化
    // connect(network.data(), &NetworkManager::WirelessNetwork::signalStrengthChanged, this, &ConnectTreeWidget::wirelessNetworkSignalChanged, Qt::UniqueConnection);

    WirelessConnectionWidget *netConnectionItem;

    if (!m_netConnectionItem[deviceUni].contains(ssid))
    {
        netConnectionItem = new WirelessConnectionWidget(deviceUni, ssid, this);
        QTreeWidgetItem *treeWidgetItem = new QTreeWidgetItem(m_netDeviceItems[deviceUni].first);
        setItemWidget(treeWidgetItem, 0, netConnectionItem);

        m_netConnectionItem[deviceUni][ssid] = qMakePair(treeWidgetItem, netConnectionItem);

        connect(netConnectionItem, &WirelessConnectionWidget::addAndActivateConnection,
                &WirelessManagerInstance, &WirelessManager::addAndActivateConnection);
        connect(netConnectionItem, &WirelessConnectionWidget::resizeShow, [this, treeWidgetItem]()
                {
                    // 要自适应行高，暂时没有好的方式
                    // TODO:需要找更合适的方式处理行高
                    // treeWidgetItem->parent()->setExpanded(false);
                    // treeWidgetItem->parent()->setExpanded(true);
                    expandAll();
                });
    }
    else
    {
        netConnectionItem = (WirelessConnectionWidget *)m_netConnectionItem[deviceUni][ssid].second;
    }

    netConnectionItem->updateStatus();
}

void netTreeWidget::wiredNetworkAppeared(const QString &deviceUni, const QString &connectionUuid)
{
    qInfo() << "netTreeWidget::wiredNetworkAppeared" << deviceUni << connectionUuid;

    updateWiredConnection(deviceUni, connectionUuid);
}

void netTreeWidget::wiredNetworkDisappeared(const QString &deviceUni, const QString &connectionUuid)
{
    qInfo() << "netTreeWidget::wiredNetworkDisappeared" << deviceUni << connectionUuid;

    removeConnection(deviceUni, connectionUuid);
}

void netTreeWidget::wirelessNetworkAppeared(const QString &deviceUni, const QString &ssid)
{
    qInfo() << "netTreeWidget::wirelessNetworkAppeared" << deviceUni << ssid;

    updateWirelessConnection(deviceUni, ssid);
}

void netTreeWidget::wirelessNetworkDisappeared(const QString &deviceUni, const QString &ssid)
{
    qInfo() << "netTreeWidget::wirelessNetworkDisappeared" << deviceUni << ssid;

    removeConnection(deviceUni, ssid);
}

void netTreeWidget::removeConnection(QString deviceUni, QString connectUuid)
{
    qInfo() << "netTreeWidget::removeConnection" << deviceUni << connectUuid;

    if (m_netConnectionItem.contains(deviceUni) && m_netConnectionItem[deviceUni].contains(connectUuid))
    {
        QTreeWidgetItem *item = m_netConnectionItem[deviceUni][connectUuid].first;
        QWidget *widget = m_netConnectionItem[deviceUni][connectUuid].second;
        item->parent()->removeChild(item);

        delete item;
        delete widget;

        m_netConnectionItem[deviceUni].remove(connectUuid);
    }
}

void netTreeWidget::activeConnectionChanged()
{
    qInfo() << "netTreeWidget::activeConnectionChanged";

    for (auto connection : m_netConnectionItem)
    {
        for (auto itemWithWidget : connection)
        {
            if (NetworkManager::Device::Type::Ethernet == m_netType)
            {
                auto *item = (WiredConnectionWidget *)itemWithWidget.second;
                item->updateStatus();
            }
            else if (NetworkManager::Device::Type::Wifi == m_netType)
            {
                auto *item = (WirelessConnectionWidget *)itemWithWidget.second;
                item->updateStatus();
            }
        }
    }
}

}  // namespace HwConf
}  // namespace Kiran
