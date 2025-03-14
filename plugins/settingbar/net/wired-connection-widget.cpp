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

#include <kiran-log/qt5-log-i.h>
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/Settings>

#include "lib/common/notify.h"
#include "net-common.h"
#include "ui_wired-connection-widget.h"
#include "wired-connection-widget.h"

namespace Kiran
{
namespace SettingBar
{
WiredConnectionWidget::WiredConnectionWidget(QString deviceUni, QString connectionUuid, QWidget *parent)
    : QWidget(parent),
      m_ui(new Ui::WiredConnectionWidget),
      m_deviceUni(deviceUni),
      m_connectionUuid(connectionUuid)
{
    m_ui->setupUi(this);

    resetStatus();
}

WiredConnectionWidget::~WiredConnectionWidget()
{
    delete m_ui;
}

void WiredConnectionWidget::updateStatus()
{
    auto device = NetworkManager::findNetworkInterface(m_deviceUni);
    NetworkManager::ActiveConnection::State state = NetworkManager::ActiveConnection::Deactivated;
    auto activeConnection = device->activeConnection();
    if (activeConnection)
    {
        if (activeConnection->uuid() == m_connectionUuid)
        {
            state = activeConnection->state();
        }
    }

    NetworkManager::Connection::Ptr connection = NetworkManager::findConnectionByUuid(m_connectionUuid);
    QString connectionName = connection->name();
    m_ui->labelName->setText(connectionName);

    bool connectedFlag = m_isConnected;
    setActiveStatus(state);
    if (!m_firstUpdateFlag && connectedFlag != m_isConnected)
    {
        // 连接状态变化通知
        Common::generalNotify(tr("wired network"), connectionName + " " + (m_isConnected ? tr("connected") : tr("disconnected")));
    }
    m_firstUpdateFlag = false;
}

void WiredConnectionWidget::setActiveStatus(NetworkManager::ActiveConnection::State state)
{
    KLOG_INFO() << "WiredConnectionWidget::setActiveStatus" << state;
    switch (state)
    {
    case NetworkManager::ActiveConnection::State::Activating:
    case NetworkManager::ActiveConnection::State::Deactivating:
        // 载入状态
        m_ui->labelLoading->setVisible(true);
        m_ui->toolButtonConnectStatu->setVisible(false);
        break;
    case NetworkManager::ActiveConnection::State::Activated:
        m_ui->labelLoading->setVisible(false);
        m_ui->toolButtonConnectStatu->setVisible(true);
        m_isConnected = true;
        break;
    default:
        // 重置状态
        resetStatus();
        break;
    }

    m_ui->labelInfo->clear();
}

void WiredConnectionWidget::resetStatus()
{
    m_ui->labelLoading->setVisible(false);
    m_ui->toolButtonConnectStatu->setVisible(false);
    m_isConnected = false;
}

void WiredConnectionWidget::mouseDoubleClickEvent(QMouseEvent *event)
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

}  // namespace SettingBar
}  // namespace Kiran
