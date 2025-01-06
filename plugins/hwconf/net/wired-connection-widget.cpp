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

#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/Settings>

#include "net-common.h"
#include "ui_wired-connection-widget.h"
#include "wired-connection-widget.h"

namespace Kiran
{
namespace HwConf
{
WiredConnectionWidget::WiredConnectionWidget(QString deviceUni, QString connectionUuid, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::WiredConnectionWidget),
      m_deviceUni(deviceUni),
      m_connectionUuid(connectionUuid),
      m_isConnected(false)
{
    ui->setupUi(this);
}

WiredConnectionWidget::~WiredConnectionWidget()
{
    delete ui;
}

void WiredConnectionWidget::updateStatus()
{
    NetworkManager::Device::Ptr device = NetworkManager::findNetworkInterface(m_deviceUni);
    auto activeConnection = device->activeConnection();
    NetworkManager::Connection::Ptr connection = NetworkManager::findConnectionByUuid(m_connectionUuid);
    QString connectionName = connection->name();
    QString activeConnectionUuid = activeConnection ? activeConnection->uuid() : "";
    if (activeConnectionUuid == m_connectionUuid)
    {
        m_isConnected = true;
    }
    else
    {
        m_isConnected = false;
    }

    ui->labelName->setText(connectionName);
    ui->labelInfo->clear();
    if (m_isConnected)
    {
        ui->toolButtonConnectStatu->setVisible(true);
        ui->pushButtonConnect->setText(tr("disconnect"));
    }
    else
    {
        ui->toolButtonConnectStatu->setVisible(false);
        ui->pushButtonConnect->setText(tr("connect"));
    }
}

void WiredConnectionWidget::on_pushButtonConnect_clicked()
{
    if (m_isConnected)
    {
        NetCommon::deactivateConnection(m_connectionUuid);
    }
    else
    {
        NetCommon::activateConnection(m_deviceUni, m_connectionUuid);
    }
}

}  // namespace HwConf
}  // namespace Kiran
