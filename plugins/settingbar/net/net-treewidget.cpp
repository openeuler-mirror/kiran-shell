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
#include "lib/common/notify.h"
#include "net-common.h"
#include "net-treewidget.h"
#include "wired-connection-widget.h"
#include "wired-manager.h"
#include "wireless-connection-widget.h"
#include "wireless-manager.h"

enum
{
    ROW_HEIGHT = 40,
    ICON_SIZE = 24,
    ICON_TEXT_MARGIN = 12,
    INDENTATION = 10
};

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
    QPalette palette = this->palette();
    palette.setBrush(QPalette::Base, QBrush(QColor(0, 0, 0, 0)));
    setPalette(palette);

    // 网络信号
    if (NetworkManager::Device::Type::Ethernet == m_netType)
    {
        connect(&WiredManagerInstance, &WiredManager::deviceListChanged, this, &netTreeWidget::updateDeviceList);
        // 有线接入点变化信号
        connect(&WiredManagerInstance, &WiredManager::availableConnectionAppeared, this, &netTreeWidget::wiredNetworkAppeared);
        connect(&WiredManagerInstance, &WiredManager::availableConnectionDisappeared, this, &netTreeWidget::wiredNetworkDisappeared);
        connect(&WiredManagerInstance, &WiredManager::deviceStateChanged, this, &netTreeWidget::updateDeviceStatus);
        connect(&WiredManagerInstance, &WiredManager::activeConnectionStateChanged, this, &netTreeWidget::activeConnectionStateChanged);
    }
    else if (NetworkManager::Device::Type::Wifi == m_netType)
    {
        connect(&WirelessManagerInstance, &WirelessManager::deviceListChanged, this, &netTreeWidget::updateDeviceList);
        // 无线接入点变化信号
        connect(&WirelessManagerInstance, &WirelessManager::networkAppeared, this, &netTreeWidget::wirelessNetworkAppeared);
        connect(&WirelessManagerInstance, &WirelessManager::networkDisappeared, this, &netTreeWidget::wirelessNetworkDisappeared);
        connect(&WirelessManagerInstance, &WirelessManager::deviceStateChanged, this, &netTreeWidget::updateDeviceStatus);
        connect(&WirelessManagerInstance, &WirelessManager::activeConnectionStateChanged, this, &netTreeWidget::activeConnectionStateChanged);
        // 被动请求密码信号
        connect(&WirelessManagerInstance, &WirelessManager::requestPassword, this, &netTreeWidget::requestPassword);
    }

    // 初始化网卡列表
    updateDeviceList();
    expandAll();
}

void netTreeWidget::updateDeviceList()
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
        // 若网卡不存在，则添加网卡设备
        // 若网卡存在，则更新网卡设备
        updateNetDevice(uni);
    }

    // 移除不存在的网卡
    for (const auto &uni : m_deviceItems.keys())
    {
        if (!deviceUnis.contains(uni))
        {
            QTreeWidgetItem *item = m_deviceItems[uni].first;
            QWidget *widget = m_deviceItems[uni].second;
            takeTopLevelItem(indexOfTopLevelItem(item));

            delete item;
            delete widget;

            m_deviceItems.remove(uni);
        }
    }
}

void netTreeWidget::updateNetDevice(const QString &deviceUni)
{
    KLOG_INFO(LCSettingbar) << "netTreeWidget::updateNetDevice" << deviceUni;

    if (!m_deviceItems.contains(deviceUni))
    {
        auto *netDeviceItem = new DeviceWidget(m_netType, deviceUni, this);
        auto *treeWidgetItem = new QTreeWidgetItem(this);
        setItemWidget(treeWidgetItem, 0, netDeviceItem);
        treeWidgetItem->setExpanded(true);  // 默认展开

        m_deviceItems[deviceUni] = qMakePair(treeWidgetItem, netDeviceItem);

        netDeviceItem->Init();
    }

    auto device = NetworkManager::findNetworkInterface(deviceUni);
    updateDeviceItem(deviceUni, device->state());

    QString connecttionUuids;
    if (NetworkManager::Device::Type::Ethernet == m_netType)
    {
        // 添加有线连接
        NetworkManager::Connection::List connections = device->availableConnections();
        for (const auto &connection : connections)
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
    auto connectionMap = m_connectionItems[deviceUni];
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
    if (!m_connectionItems[deviceUni].contains(connectionUuid))
    {
        netConnectionItem = new WiredConnectionWidget(deviceUni, connectionUuid, this);
        auto *treeWidgetItem = new QTreeWidgetItem(m_deviceItems[deviceUni].first);
        setItemWidget(treeWidgetItem, 0, netConnectionItem);

        m_connectionItems[deviceUni][connectionUuid] = qMakePair(treeWidgetItem, netConnectionItem);
    }
    else
    {
        netConnectionItem = (WiredConnectionWidget *)m_connectionItems[deviceUni][connectionUuid].second;
    }

    netConnectionItem->updateStatus();
}

void netTreeWidget::updateWirelessConnection(const QString &deviceUni, const QString &ssid)
{
    WirelessConnectionWidget *netConnectionItem;

    if (!m_connectionItems[deviceUni].contains(ssid))
    {
        netConnectionItem = new WirelessConnectionWidget(deviceUni, ssid, this);
        auto *treeWidgetItem = new QTreeWidgetItem(m_deviceItems[deviceUni].first);
        setItemWidget(treeWidgetItem, 0, netConnectionItem);

        m_connectionItems[deviceUni][ssid] = qMakePair(treeWidgetItem, netConnectionItem);

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
        netConnectionItem = (WirelessConnectionWidget *)m_connectionItems[deviceUni][ssid].second;
    }

    netConnectionItem->updateStatus();
}

void netTreeWidget::wiredNetworkAppeared(const QString &deviceUni, const QString &connectionUuid)
{
    KLOG_DEBUG(LCSettingbar) << "wiredNetworkAppeared" << deviceUni << connectionUuid;

    updateWiredConnection(deviceUni, connectionUuid);
}

void netTreeWidget::wiredNetworkDisappeared(const QString &deviceUni)
{
    KLOG_DEBUG(LCSettingbar) << "wiredNetworkDisappeared" << deviceUni;

    removeConnection(deviceUni);
}

void netTreeWidget::wirelessNetworkAppeared(const QString &deviceUni, const QString &ssid)
{
    KLOG_DEBUG(LCSettingbar) << "wirelessNetworkAppeared" << deviceUni << ssid;

    updateWirelessConnection(deviceUni, ssid);
}

void netTreeWidget::wirelessNetworkDisappeared(const QString &deviceUni, const QString &ssid)
{
    KLOG_DEBUG(LCSettingbar) << "wirelessNetworkDisappeared" << deviceUni << ssid;

    removeConnection(deviceUni, ssid);
}

void netTreeWidget::removeConnection(const QString &deviceUni, const QString &connectUuid)
{
    KLOG_DEBUG(LCSettingbar) << "netTreeWidget::removeConnection" << deviceUni << connectUuid;

    // 若connectUuid为空，则先遍历获取所有连接uuid，然后移除不存在的连接
    if (connectUuid.isEmpty())
    {
        QString connecttionUuids;
        if (NetworkManager::Device::Type::Ethernet == m_netType)
        {
            auto device = NetworkManager::findNetworkInterface(deviceUni);
            for (const auto &connection : device->availableConnections())
            {
                connecttionUuids.append(connection->uuid());
            }
        }
        else if (NetworkManager::Device::Type::Wifi == m_netType)
        {
            auto networkInfos = WirelessManagerInstance.getNetworkInfoList(deviceUni);
            for (const auto &networkInfo : networkInfos)
            {
                connecttionUuids.append(networkInfo.ssid);
            }
        }

        // 移除不存在的连接
        auto connectionMap = m_connectionItems[deviceUni];
        for (const QString &connectUuid : connectionMap.keys())
        {
            if (!connecttionUuids.contains(connectUuid))
            {
                removeConnection(deviceUni, connectUuid);
            }
        }
    }
    else
    {
        if (m_connectionItems.contains(deviceUni) && m_connectionItems[deviceUni].contains(connectUuid))
        {
            QTreeWidgetItem *item = m_connectionItems[deviceUni][connectUuid].first;
            QWidget *widget = m_connectionItems[deviceUni][connectUuid].second;
            item->parent()->removeChild(item);

            delete item;
            delete widget;

            m_connectionItems[deviceUni].remove(connectUuid);
        }
    }
}

void netTreeWidget::updateDeviceStatus(const QString &deviceUni, NetworkManager::Device::State state, NetworkManager::Device::StateChangeReason reason)
{
    if (!m_connectionItems.contains(deviceUni))
    {
        KLOG_ERROR(LCSettingbar) << "!m_netConnectionItem.contains(deviceUni)" << deviceUni;
        return;
    }

    KLOG_INFO(LCSettingbar) << "updateActiveStatus" << deviceUni << state << reason;
    auto device = NetworkManager::findNetworkInterface(deviceUni);
    notifyDeviceState(device, state, reason);

    updateDeviceItem(deviceUni, state);

    for (const auto &connection : m_connectionItems[deviceUni])
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

void netTreeWidget::updateDeviceItem(const QString &deviceUni, NetworkManager::Device::State state)
{
    if (m_deviceItems.contains(deviceUni))
    {
        m_deviceItems[deviceUni].first->setHidden(!(state >= NetworkManager::Device::Unmanaged));
        m_deviceItems[deviceUni].second->setUnavailable(state == NetworkManager::Device::Unavailable);
    }
}

void netTreeWidget::activeConnectionStateChanged(const QString &deviceUni, NetworkManager::ActiveConnection::State state)
{
    if (!m_connectionItems.contains(deviceUni))
    {
        KLOG_ERROR(LCSettingbar) << "!m_netConnectionItem.contains(deviceUni)" << deviceUni;
        return;
    }

    KLOG_INFO(LCSettingbar) << "activeConnectionStateChanged" << deviceUni << state;

    for (auto connection : m_connectionItems[deviceUni])
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
    KLOG_DEBUG(LCSettingbar) << "netTreeWidget::requestPassword" << devicePath << ssid << wait;

    if (m_connectionItems.contains(devicePath) && m_connectionItems[devicePath].contains(ssid))
    {
        auto *netConnectionItem = (WirelessConnectionWidget *)m_connectionItems[devicePath][ssid].second;
        netConnectionItem->requestPassword();
    }
}

void netTreeWidget::notifyDeviceState(const NetworkManager::Device::Ptr &device, NetworkManager::Device::State state, NetworkManager::Device::StateChangeReason reason)
{
    KLOG_INFO(LCSettingbar) << "notifyDeviceState" << state << reason;

    const QString title = NetCommon::prettyInterfaceName(device->type(), device->interfaceName());

    QString text;
    if (state == NetworkManager::Device::Unavailable || state == NetworkManager::Device::Failed)
    {
        switch (reason)
        {
        case NetworkManager::Device::NoReason:
        case NetworkManager::Device::UnknownReason:
        case NetworkManager::Device::NowManagedReason:
        case NetworkManager::Device::NowUnmanagedReason:
            return;
        case NetworkManager::Device::ConfigFailedReason:
            text = tr("The device could not be configured");
            break;
        case NetworkManager::Device::ConfigUnavailableReason:
            text = tr("IP configuration was unavailable");
            break;
        case NetworkManager::Device::ConfigExpiredReason:
            text = tr("IP configuration expired");
            break;
        case NetworkManager::Device::NoSecretsReason:
            text = tr("No secrets were provided");
            break;
        case NetworkManager::Device::AuthSupplicantDisconnectReason:
            text = tr("Authorization supplicant disconnected");
            break;
        case NetworkManager::Device::AuthSupplicantConfigFailedReason:
            text = tr("Authorization supplicant's configuration failed");
            break;
        case NetworkManager::Device::AuthSupplicantFailedReason:
            text = tr("Authorization supplicant failed");
            break;
        case NetworkManager::Device::AuthSupplicantTimeoutReason:
            text = tr("Authorization supplicant timed out");
            break;
        case NetworkManager::Device::PppStartFailedReason:
            text = tr("PPP failed to start");
            break;
        case NetworkManager::Device::PppDisconnectReason:
            text = tr("PPP disconnected");
            break;
        case NetworkManager::Device::PppFailedReason:
            text = tr("PPP failed");
            break;
        case NetworkManager::Device::DhcpStartFailedReason:
            text = tr("DHCP failed to start");
            break;
        case NetworkManager::Device::DhcpErrorReason:
            text = tr("A DHCP error occurred");
            break;
        case NetworkManager::Device::DhcpFailedReason:
            text = tr("DHCP failed");
            break;
        case NetworkManager::Device::SharedStartFailedReason:
            text = tr("The shared service failed to start");
            break;
        case NetworkManager::Device::SharedFailedReason:
            text = tr("The shared service failed");
            break;
        case NetworkManager::Device::AutoIpStartFailedReason:
            text = tr("The auto IP service failed to start");
            break;
        case NetworkManager::Device::AutoIpErrorReason:
            text = tr("The auto IP service reported an error");
            break;
        case NetworkManager::Device::AutoIpFailedReason:
            text = tr("The auto IP service failed");
            break;
        case NetworkManager::Device::ModemBusyReason:
            text = tr("The modem is busy");
            break;
        case NetworkManager::Device::ModemNoDialToneReason:
            text = tr("The modem has no dial tone");
            break;
        case NetworkManager::Device::ModemNoCarrierReason:
            text = tr("The modem shows no carrier");
            break;
        case NetworkManager::Device::ModemDialTimeoutReason:
            text = tr("The modem dial timed out");
            break;
        case NetworkManager::Device::ModemDialFailedReason:
            text = tr("The modem dial failed");
            break;
        case NetworkManager::Device::ModemInitFailedReason:
            text = tr("The modem could not be initialized");
            break;
        case NetworkManager::Device::GsmApnSelectFailedReason:
            text = tr("The GSM APN could not be selected");
            break;
        case NetworkManager::Device::GsmNotSearchingReason:
            text = tr("The GSM modem is not searching");
            break;
        case NetworkManager::Device::GsmRegistrationDeniedReason:
            text = tr("GSM network registration was denied");
            break;
        case NetworkManager::Device::GsmRegistrationTimeoutReason:
            text = tr("GSM network registration timed out");
            break;
        case NetworkManager::Device::GsmRegistrationFailedReason:
            text = tr("GSM registration failed");
            break;
        case NetworkManager::Device::GsmPinCheckFailedReason:
            text = tr("The GSM PIN check failed");
            break;
        case NetworkManager::Device::FirmwareMissingReason:
            text = tr("Device firmware is missing");
            break;
        case NetworkManager::Device::DeviceRemovedReason:
            text = tr("The device was removed");
            break;
        case NetworkManager::Device::SleepingReason:
            text = tr("The networking system is now sleeping");
            break;
        case NetworkManager::Device::ConnectionRemovedReason:
            text = tr("The connection was removed");
            break;
        case NetworkManager::Device::UserRequestedReason:
            return;
        case NetworkManager::Device::CarrierReason:
            text = tr("The cable was disconnected");
            break;
        case NetworkManager::Device::ConnectionAssumedReason:
        case NetworkManager::Device::SupplicantAvailableReason:
            return;
        case NetworkManager::Device::ModemNotFoundReason:
            text = tr("The modem could not be found");
            break;
        case NetworkManager::Device::BluetoothFailedReason:
            text = tr("The bluetooth connection failed or timed out");
            break;
        case NetworkManager::Device::GsmSimNotInserted:
            text = tr("GSM Modem's SIM Card not inserted");
            break;
        case NetworkManager::Device::GsmSimPinRequired:
            text = tr("GSM Modem's SIM Pin required");
            break;
        case NetworkManager::Device::GsmSimPukRequired:
            text = tr("GSM Modem's SIM Puk required");
            break;
        case NetworkManager::Device::GsmSimWrong:
            text = tr("GSM Modem's SIM wrong");
            break;
        case NetworkManager::Device::InfiniBandMode:
            text = tr("InfiniBand device does not support connected mode");
            break;
        case NetworkManager::Device::DependencyFailed:
            text = tr("A dependency of the connection failed");
            break;
        case NetworkManager::Device::Br2684Failed:
            text = tr("Problem with the RFC 2684 Ethernet over ADSL bridge");
            break;
        case NetworkManager::Device::ModemManagerUnavailable:
            text = tr("ModemManager not running");
            break;
        case NetworkManager::Device::SsidNotFound:
            return;
        case NetworkManager::Device::SecondaryConnectionFailed:
            text = tr("A secondary connection of the base connection failed");
            break;
        case NetworkManager::Device::DcbFcoeFailed:
            text = tr("DCB or FCoE setup failed");
            break;
        case NetworkManager::Device::TeamdControlFailed:
            text = tr("teamd control failed");
            break;
        case NetworkManager::Device::ModemFailed:
            text = tr("Modem failed or no longer available");
            break;
        case NetworkManager::Device::ModemAvailable:
            text = tr("Modem now ready and available");
            break;
        case NetworkManager::Device::SimPinIncorrect:
            text = tr("The SIM PIN was incorrect");
            break;
        case NetworkManager::Device::NewActivation:
            text = tr("A new connection activation was enqueued");
            break;
        case NetworkManager::Device::ParentChanged:
            text = tr("The device's parent changed");
            break;
        case NetworkManager::Device::ParentManagedChanged:
            text = tr("The device parent's management changed");
            break;
        case NetworkManager::Device::Reserved:
            return;
        }
    }

    KLOG_INFO(LCSettingbar) << "notifyDeviceState" << title << text;

    if (!text.isEmpty())
    {
        Common::generalNotify(title, text);
    }
}

}  // namespace SettingBar
}  // namespace Kiran
