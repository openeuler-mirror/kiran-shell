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

#include <kiran-integration/theme/palette.h>
#include <qt5-log-i.h>
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/Settings>
#include <QHeaderView>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QStyledItemDelegate>

#include "device-widget.h"
#include "lib/common/logging-category.h"
#include "net-common.h"
#include "net-treewidget.h"
#include "wired-connection-widget.h"
#include "wired-manager.h"
#include "wireless-connection-widget.h"
#include "wireless-manager.h"

#define ROW_HEIGHT 40
#define ICON_SIZE 24
#define ICON_TEXT_MARGIN 12
#define INDENTATION 10

namespace Kiran
{
class ItemDelegate : public QStyledItemDelegate
{
public:
    ItemDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
    {
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QSize size = QStyledItemDelegate::sizeHint(option, index);
        size.setHeight(ROW_HEIGHT);
        return size;
    };
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        auto *palette = Kiran::Theme::Palette::getDefault();

        // 选中底色
        if (option.state & QStyle::State_Selected)
        {
            QColor bgColor = palette->getColor(Kiran::Theme::Palette::SUNKEN, Kiran::Theme::Palette::WIDGET);

            painter->fillRect(option.rect, bgColor);
        }
        // 鼠标移入底色
        if (option.state & QStyle::State_MouseOver)
        {
            QColor bgColor = palette->getColor(Kiran::Theme::Palette::MOUSE_OVER, Kiran::Theme::Palette::WIDGET);
            painter->fillRect(option.rect, bgColor);
        }
    }
};

namespace SettingBar
{
netTreeWidget::netTreeWidget(NetworkManager::Device::Type deviceType, QWidget *parent)
    : QTreeWidget(parent),
      m_netType(deviceType)
{
    // 表头隐藏
    setHeaderHidden(true);
    // 为了绘制底色时，区域为完整一行，设置缩进为0，在绘制中加入缩进
    setIndentation(0);
    setMouseTracking(true);
    setRootIsDecorated(true);

    // 样式代理
    auto *itemDelegate = new ItemDelegate(this);
    setItemDelegate(itemDelegate);

    // 背景透明
    QPalette p = this->palette();
    p.setBrush(QPalette::Base, QBrush(QColor(0, 0, 0, 0)));
    setPalette(p);

    // 网络信号
    if (NetworkManager::Device::Type::Ethernet == m_netType)
    {
        // 有线接入点变化信号
        connect(&WiredManagerInstance, &WiredManager::availableConnectionAppeared, this, &netTreeWidget::wiredNetworkAppeared);
        connect(&WiredManagerInstance, &WiredManager::availableConnectionDisappeared, this, &netTreeWidget::wiredNetworkDisappeared);
        connect(&WiredManagerInstance, &WiredManager::stateChanged, this, &netTreeWidget::updateActiveStatus);
    }
    else if (NetworkManager::Device::Type::Wifi == m_netType)
    {
        // 无线接入点变化信号
        connect(&WirelessManagerInstance, &WirelessManager::networkAppeared, this, &netTreeWidget::wirelessNetworkAppeared);
        connect(&WirelessManagerInstance, &WirelessManager::networkDisappeared, this, &netTreeWidget::wirelessNetworkDisappeared);
        // 被动请求密码信号
        connect(&WirelessManagerInstance, &WirelessManager::requestPassword, this, &netTreeWidget::requestPassword);
        connect(&WirelessManagerInstance, &WirelessManager::stateChanged, this, &netTreeWidget::updateActiveStatus);
        connect(&WirelessManagerInstance, &WirelessManager::activeConnectionStateChanged, this, &netTreeWidget::activeConnectionStateChanged);
    }

    // 初始化网络状态
    updateNetworkStatus();
    expandAll();
}

void netTreeWidget::updateNetworkStatus()
{
    KLOG_INFO(LCSettingbar) << "ConnectTreeWidget::updateNetworkStatus";

    QStringList deviceUnis;
    if (NetworkManager::Device::Type::Ethernet == m_netType)
    {
        deviceUnis = WiredManagerInstance.getDevices();
    }
    else if (NetworkManager::Device::Type::Wifi == m_netType)
    {
        deviceUnis = WirelessManagerInstance.getDevices();
    }
    for (const auto &uni : deviceUnis)
    {
        // 添加网卡设备
        updateNetDevice(uni);
    }

    // 移除不存在的网卡
    for (const auto &uni : m_netDeviceItems.keys())
    {
        if (!deviceUnis.contains(uni))
        {
            QTreeWidgetItem *item = m_netDeviceItems[uni].first;
            QWidget *widget = m_netDeviceItems[uni].second;
            takeTopLevelItem(indexOfTopLevelItem(item));

            delete item;
            delete widget;

            m_netDeviceItems.remove(uni);
        }
    }
}

void netTreeWidget::updateNetDevice(const QString &deviceUni)
{
    KLOG_INFO(LCSettingbar) << "netTreeWidget::updateNetDevice" << deviceUni;

    if (!m_netDeviceItems.contains(deviceUni))
    {
        auto *netDeviceItem = new DeviceWidget(m_netType, deviceUni, this);
        auto *treeWidgetItem = new QTreeWidgetItem(this);
        setItemWidget(treeWidgetItem, 0, netDeviceItem);

        m_netDeviceItems[deviceUni] = qMakePair(treeWidgetItem, netDeviceItem);

        netDeviceItem->Init();
    }

    QString connecttionUuids;
    if (NetworkManager::Device::Type::Ethernet == m_netType)
    {
        // 添加有线连接
        auto device = NetworkManager::findNetworkInterface(deviceUni);
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
        for (const auto &networkInfo : networkInfos)
        {
            updateWirelessConnection(deviceUni, networkInfo.ssid);
            connecttionUuids.append(networkInfo.ssid);
        }
    }

    // 移除不存在的连接
    auto connectionMap = m_netConnectionItem[deviceUni];
    for (const QString &connectUuid : connectionMap.keys())
    {
        if (!connecttionUuids.contains(connectUuid))
        {
            removeConnection(deviceUni, connectUuid);
        }
    }
}

void netTreeWidget::updateWiredConnection(const QString &deviceUni, const QString &connectionUuid)
{
    KLOG_INFO(LCSettingbar) << "netTreeWidget::updateNetConnection" << deviceUni << connectionUuid;

    WiredConnectionWidget *netConnectionItem;
    if (!m_netConnectionItem[deviceUni].contains(connectionUuid))
    {
        netConnectionItem = new WiredConnectionWidget(deviceUni, connectionUuid, this);
        auto *treeWidgetItem = new QTreeWidgetItem(m_netDeviceItems[deviceUni].first);
        setItemWidget(treeWidgetItem, 0, netConnectionItem);

        m_netConnectionItem[deviceUni][connectionUuid] = qMakePair(treeWidgetItem, netConnectionItem);
    }
    else
    {
        netConnectionItem = (WiredConnectionWidget *)m_netConnectionItem[deviceUni][connectionUuid].second;
    }

    netConnectionItem->updateStatus();
}

void netTreeWidget::updateWirelessConnection(const QString &deviceUni, const QString &ssid)
{
    WirelessConnectionWidget *netConnectionItem;

    if (!m_netConnectionItem[deviceUni].contains(ssid))
    {
        netConnectionItem = new WirelessConnectionWidget(deviceUni, ssid, this);
        auto *treeWidgetItem = new QTreeWidgetItem(m_netDeviceItems[deviceUni].first);
        setItemWidget(treeWidgetItem, 0, netConnectionItem);

        m_netConnectionItem[deviceUni][ssid] = qMakePair(treeWidgetItem, netConnectionItem);

        connect(netConnectionItem, &WirelessConnectionWidget::respondPasswdRequest,
                &WirelessManagerInstance, &WirelessManager::respondPasswdRequest);
        connect(netConnectionItem, &WirelessConnectionWidget::addAndActivateNetwork,
                &WirelessManagerInstance, &WirelessManager::addAndActivateNetwork);
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
    KLOG_INFO(LCSettingbar) << "wiredNetworkAppeared" << deviceUni << connectionUuid;

    updateWiredConnection(deviceUni, connectionUuid);
}

void netTreeWidget::wiredNetworkDisappeared(const QString &deviceUni, const QString &connectionUuid)
{
    KLOG_INFO(LCSettingbar) << "wiredNetworkDisappeared" << deviceUni << connectionUuid;

    removeConnection(deviceUni, connectionUuid);
}

void netTreeWidget::wirelessNetworkAppeared(const QString &deviceUni, const QString &ssid)
{
    KLOG_INFO(LCSettingbar) << "wirelessNetworkAppeared" << deviceUni << ssid;

    updateWirelessConnection(deviceUni, ssid);
}

void netTreeWidget::wirelessNetworkDisappeared(const QString &deviceUni, const QString &ssid)
{
    KLOG_INFO(LCSettingbar) << "wirelessNetworkDisappeared" << deviceUni << ssid;

    removeConnection(deviceUni, ssid);
}

void netTreeWidget::removeConnection(const QString &deviceUni, const QString &connectUuid)
{
    KLOG_INFO(LCSettingbar) << "netTreeWidget::removeConnection" << deviceUni << connectUuid;

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

void netTreeWidget::updateActiveStatus(const QString &deviceUni, NetworkManager::Device::State state)
{
    if (!m_netConnectionItem.contains(deviceUni))
    {
        KLOG_ERROR(LCSettingbar) << "!m_netConnectionItem.contains(deviceUni)" << deviceUni;
        return;
    }

    for (const auto &connection : m_netConnectionItem)
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

void netTreeWidget::activeConnectionStateChanged(const QString &deviceUni, NetworkManager::ActiveConnection::State state)
{
    if (!m_netConnectionItem.contains(deviceUni))
    {
        KLOG_ERROR(LCSettingbar) << "!m_netConnectionItem.contains(deviceUni)" << deviceUni;
        return;
    }

    KLOG_INFO(LCSettingbar) << "activeConnectionStateChanged" << deviceUni << state;

    for (auto connection : m_netConnectionItem[deviceUni])
    {
        if (NetworkManager::Device::Type::Ethernet == m_netType)
        {
            auto *item = (WiredConnectionWidget *)connection.second;
            item->updateStatus();
        }
        else if (NetworkManager::Device::Type::Wifi == m_netType)
        {
            auto *item = (WirelessConnectionWidget *)connection.second;
            item->updateStatus();
        }
    }
}

void netTreeWidget::requestPassword(const QString &devicePath, const QString &ssid, bool wait)
{
    KLOG_INFO(LCSettingbar) << "netTreeWidget::requestPassword" << devicePath << ssid << wait;

    if (m_netConnectionItem.contains(devicePath) && m_netConnectionItem[devicePath].contains(ssid))
    {
        auto *netConnectionItem = (WirelessConnectionWidget *)m_netConnectionItem[devicePath][ssid].second;
        netConnectionItem->requestPassword();
    }
}

}  // namespace SettingBar
}  // namespace Kiran
