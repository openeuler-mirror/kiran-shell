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
#include <NetworkManagerQt/AccessPoint>
#include <NetworkManagerQt/Utils>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/WirelessSecuritySetting>
#include <NetworkManagerQt/WirelessSetting>

#include "ui_wireless-connection-widget.h"
#include "wireless-connection-widget.h"

namespace Kiran
{
namespace HwConf
{
WirelessConnectionWidget::WirelessConnectionWidget(QString deviceUni, QString ssid, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::WirelessConnectionWidget),
      m_deviceUni(deviceUni),
      m_ssid(ssid),
      m_isConnected(false)
{
    ui->setupUi(this);
    ui->lineEditSsid->setVisible(false);
    ui->lineEditPassword->setVisible(false);

    ui->labelName->setText(ssid);
    ui->labelInfo->clear();
}

WirelessConnectionWidget::~WirelessConnectionWidget()
{
    delete ui;
}

void WirelessConnectionWidget::updateStatus()
{
    auto device = NetworkManager::findNetworkInterface(m_deviceUni);
    NetworkManager::WirelessDevice::Ptr wirelessDevice = device.objectCast<NetworkManager::WirelessDevice>();
    NetworkManager::AccessPoint::Ptr activeAccessPoint = wirelessDevice->activeAccessPoint();
    QString activeSsid = activeAccessPoint ? activeAccessPoint->ssid() : "";

    ui->labelInfo->clear();
    if (activeSsid == m_ssid)
    {
        ui->toolButtonConnectStatu->setVisible(true);
        ui->pushButtonConnect->setText(tr("disconnect"));
        m_isConnected = true;
    }
    else
    {
        ui->toolButtonConnectStatu->setVisible(false);
        ui->pushButtonConnect->setText(tr("connect"));
        m_isConnected = false;
    }
    //    KLOG_INFO() << m_ssid << "----------connect state:" << m_isConnected;
}

void WirelessConnectionWidget::on_pushButtonConnect_clicked()
{
    if (m_isConnected)
    {
        auto connection = NetCommon::getAvailableConnectionBySsid(m_deviceUni, m_ssid);
        NetCommon::deactivateConnection(connection->uuid());
        return;
    }
    else
    {
        auto connection = NetCommon::getAvailableConnectionBySsid(m_deviceUni, m_ssid);
        if (!connection.isNull())
        {
            NetCommon::activateConnection(m_deviceUni, connection->uuid());
            return;
        }

        // 密码输入框未显示，则显示，让用户输入密码
        if (!ui->lineEditPassword->isVisible())
        {
            ui->lineEditPassword->setVisible(true);
            emit resizeShow();
            return;
        }

        // 密码输入框已显示，则获取输入的密码，进行连接
        QString password = ui->lineEditPassword->text();
        if (password.isEmpty())
        {
            return;
        }

        emit addAndActivateConnection(m_deviceUni, m_ssid, password);

        ui->lineEditPassword->setVisible(false);
        emit resizeShow();
    }
}
}  // namespace HwConf
}  // namespace Kiran
