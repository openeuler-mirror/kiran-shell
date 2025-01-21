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
#include <NetworkManagerQt/WirelessDevice>
#include <QTimer>

#include "device-widget.h"
#include "net-common.h"
#include "ui_device-widget.h"
#include "wireless-manager.h"

namespace Kiran
{
namespace HwConf
{
DeviceWidget::DeviceWidget(NetworkManager::Device::Type type, QString deviceUni, QWidget* parent)
    : QWidget(parent),
      ui(new Ui::DeviceWidget),
      m_deviceType(type),
      m_deviceUni(deviceUni)
{
    ui->setupUi(this);
}

DeviceWidget::~DeviceWidget()
{
    delete ui;
}

void DeviceWidget::Init()
{
    auto device = NetworkManager::findNetworkInterface(m_deviceUni);
    connect(device.data(), &NetworkManager::Device::interfaceNameChanged, this, &DeviceWidget::updateInfo);

    if (NetworkManager::Device::Type::Wifi == m_deviceType)
    {
        WirelessManagerInstance.scan(m_deviceUni);
    }
    else
    {
        ui->toolButtonScan->setVisible(false);
    }

    updateInfo();
}

void DeviceWidget::updateInfo()
{
    auto device = NetworkManager::findNetworkInterface(m_deviceUni);
    ui->labelName->setText(device->interfaceName());
}

void DeviceWidget::on_toolButtonScan_clicked()
{
    if (NetworkManager::Device::Type::Wifi == m_deviceType)
    {
        WirelessManagerInstance.scan(m_deviceUni);
    }
}

}  // namespace HwConf
}  // namespace Kiran
