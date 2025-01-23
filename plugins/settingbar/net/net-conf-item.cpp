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

#include <kiran-color-block.h>
#include <QIcon>
#include <QLabel>
#include <QSpacerItem>
#include <QVBoxLayout>

#include "ks-i.h"
#include "lib/widgets/styled-button.h"
#include "net-conf-item.h"
#include "net-treewidget.h"

namespace Kiran
{
namespace SettingBar
{
NetConfItem::NetConfItem(NetworkManager::Device::Type type, QWidget *parent)
    : SettingItem(parent),
      m_netType(type),
      m_settingWidget(nullptr)
{
    initUI();
}

NetConfItem::~NetConfItem()
{
    delete m_settingWidget;
}

void NetConfItem::init()
{
    connect(&NetCommonInstance, &NetCommon::netStatusChanged, this, &NetConfItem::updateNetworkStatus);
    connect(this, &SettingItem::iconClicked, this, &NetConfItem::netIconClicked);
    connect(this, &SettingItem::settingClicked, this, &NetConfItem::netSettingClicked);

    updateNetworkStatus();
}

void NetConfItem::initUI()
{
    m_settingWidget = new QWidget;
    auto *verticalLayout = new QVBoxLayout;
    auto *horizontalLayout = new QHBoxLayout;
    auto *verticalLayoutItems = new QVBoxLayout;
    auto *horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    auto *connectTreeBg = new KiranColorBlock(m_settingWidget);
    m_connectTreeWidget = new netTreeWidget(m_netType, connectTreeBg);
    m_backBtn = new StyledButton(m_settingWidget);
    auto *titlelabel = new QLabel(m_settingWidget);

    m_settingWidget->setLayout(verticalLayout);
    verticalLayout->setMargin(0);
    verticalLayout->addLayout(horizontalLayout);
    verticalLayout->addWidget(connectTreeBg);
    verticalLayout->setAlignment(Qt::AlignTop);
    connectTreeBg->setLayout(verticalLayoutItems);

    horizontalLayout->setMargin(0);
    horizontalLayout->addWidget(m_backBtn);
    horizontalLayout->addWidget(titlelabel);
    horizontalLayout->addSpacerItem(horizontalSpacer);
    verticalLayoutItems->addWidget(m_connectTreeWidget);

    m_backBtn->setIcon(QIcon::fromTheme(KS_ICON_HWCONF_SETTING_BACK));
    switch (m_netType)
    {
    case NetworkManager::Device::Type::Ethernet:
        titlelabel->setText(tr("Wired network"));
        setIcon(QIcon::fromTheme(KS_ICON_WIRED));
        break;
    case NetworkManager::Device::Type::Wifi:
        titlelabel->setText(tr("Wireless network"));
        setIcon(QIcon::fromTheme(KS_ICON_WIRELESS));
        break;
    default:
        break;
    }

    connect(m_backBtn, &QToolButton::clicked, this, &SettingItem::requestExitOnlyShow);
}

void NetConfItem::updateNetworkStatus()
{
    // TODO:换底色，激活时蓝底，未激活透明
    bool isActive = false;
    NetworkManager::Device::List devices;
    switch (m_netType)
    {
    case NetworkManager::Device::Type::Ethernet:
        devices = NetCommon::getEthernetDevices();
        break;
    case NetworkManager::Device::Type::Wifi:
        devices = NetCommon::getWifiDevices();
        break;
    default:
        break;
    }

    for (const auto &device : devices)
    {
        if (NetworkManager::Device::State::Activated == device->state())
        {
            isActive = true;
        }
    }

    setActive(isActive);

    m_connectTreeWidget->updateNetworkStatus();
    emit enableNetwork(!devices.isEmpty());

    update();
}

void NetConfItem::netIconClicked()
{
    NetCommon::disconnectDevice(m_netType);
}

void NetConfItem::netSettingClicked()
{
    emit requestOnlyShow(m_settingWidget);
}
}  // namespace SettingBar
}  // namespace Kiran
