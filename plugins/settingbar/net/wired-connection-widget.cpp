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
#include "lib/common/utility.h"
#include "lib/widgets/loading-label.h"
#include "lib/widgets/styled-button.h"
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
      m_connectionUuid(connectionUuid),
      m_status(NetStatus::DISCONNECTED),
      m_firstUpdateFlag(true)
{
    m_ui->setupUi(this);

    m_connectStatu = new StyledButton(this);
    m_connectStatu->setIcon(QIcon::fromTheme("ks-net-connected"));
    m_connectStatu->setEnabled(false);
    m_connectStatu->hide();

    m_connectedIcon = QIcon::fromTheme("ks-net-connected");
    QSize iconSize = m_connectedIcon.availableSizes().isEmpty() ? QSize(24, 24) : m_connectedIcon.availableSizes().first();
    m_connectedHoverIcon = Utility::convertOpacity(m_connectedIcon.pixmap(iconSize), 0.2);

    m_ui->toolButtonDisconnect->setIcon(QIcon::fromTheme("ksvg-ks-network-disconnect"));

    m_loadingLabel = new LoadingLabel(this);
    m_loadingLabel->hide();

    m_status = NetStatus::DISCONNECTED;
    updateShowStatus();
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
    m_ui->labelName->setShowText(connectionName);

    NetStatus connectedFlag = m_status;
    setActiveStatus(state);
    if (!m_firstUpdateFlag && connectedFlag != m_status)
    {
        // 连接状态变化通知
        if (m_status == CONNECTED)
        {
            Common::generalNotify(tr("wired network"), connectionName + " " + tr("connected"));
        }
        else if (m_status == DISCONNECTED)
        {
            Common::generalNotify(tr("wired network"), connectionName + " " + tr("disconnected"));
        }
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
        m_status = NetStatus::LOADING;
        break;
    case NetworkManager::ActiveConnection::State::Activated:
        m_status = NetStatus::CONNECTED;
        break;
    default:
        m_status = NetStatus::DISCONNECTED;
        break;
    }

    updateShowStatus();
}

void WiredConnectionWidget::updateShowStatus()
{
    switch (m_status)
    {
    case NetStatus::LOADING:
        m_loadingLabel->setVisible(true);
        m_connectStatu->setVisible(false);
        m_ui->layoutNetStatu->removeWidget(m_connectStatu);
        m_ui->layoutNetStatu->addWidget(m_loadingLabel);
        m_ui->toolButtonDisconnect->hide();
        break;
    case NetStatus::CONNECTED:
        m_loadingLabel->setVisible(false);
        m_connectStatu->setIcon(m_connectedIcon);
        m_connectStatu->setVisible(true);
        m_ui->layoutNetStatu->removeWidget(m_loadingLabel);
        m_ui->layoutNetStatu->addWidget(m_connectStatu);
        m_ui->toolButtonDisconnect->show();
        break;
    default:
        m_ui->toolButtonDisconnect->hide();
        if (this->underMouse())
        {
            m_loadingLabel->setVisible(false);
            m_connectStatu->setIcon(m_connectedHoverIcon);
            m_connectStatu->setVisible(true);
            m_ui->layoutNetStatu->removeWidget(m_loadingLabel);
            m_ui->layoutNetStatu->addWidget(m_connectStatu);
        }
        else
        {
            m_loadingLabel->setVisible(false);
            m_connectStatu->setVisible(false);
            m_ui->layoutNetStatu->removeWidget(m_loadingLabel);
            m_ui->layoutNetStatu->removeWidget(m_connectStatu);
        }

        break;
    }
}

void WiredConnectionWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (NetStatus::CONNECTED == m_status)
    {
        // on_toolButtonDisconnect_clicked();
    }
    else if (NetStatus::DISCONNECTED == m_status)
    {
        NetCommon::activateConnection(m_deviceUni, m_connectionUuid);
    }
}

void WiredConnectionWidget::enterEvent(QEvent *event)
{
    updateShowStatus();
}

void WiredConnectionWidget::leaveEvent(QEvent *event)
{
    updateShowStatus();
}

void WiredConnectionWidget::on_toolButtonDisconnect_clicked()
{
    NetCommon::deactivateConnection(m_connectionUuid);
}

}  // namespace SettingBar
}  // namespace Kiran
