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
#include <NetworkManagerQt/Connection>
#include <NetworkManagerQt/Utils>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/WirelessNetwork>
#include <NetworkManagerQt/WirelessSetting>
#include <QInputDialog>
#include <QToolTip>

#include "ks-i.h"
#include "lib/common/logging-category.h"
#include "lib/common/notify.h"
#include "lib/common/utility.h"
#include "lib/widgets/loading-label.h"
#include "lib/widgets/styled-button.h"
#include "ui_wireless-connection-widget.h"
#include "wireless-connection-widget.h"
#include "wireless-manager.h"

namespace Kiran
{
namespace SettingBar
{
WirelessConnectionWidget::WirelessConnectionWidget(QString deviceUni, QString ssid, QWidget *parent)
    : QWidget(parent),
      m_ui(new Ui::WirelessConnectionWidget),
      m_deviceUni(deviceUni),
      m_ssid(ssid),
      m_status(NetStatus::DISCONNECTED)
{
    m_ui->setupUi(this);

    m_connectStatu = new StyledButton(this);
    m_connectStatu->setEnabled(false);
    m_connectStatu->hide();

    m_loadingLabel = new LoadingLabel(this);
    m_loadingLabel->hide();

    m_ui->toolButtonDisconnect->setIcon(QIcon::fromTheme("ksvg-ks-network-disconnect"));

    m_connectedIcon = QIcon::fromTheme("ks-net-connected");
    QSize iconSize = m_connectedIcon.availableSizes().isEmpty() ? QSize(24, 24) : m_connectedIcon.availableSizes().first();
    m_connectedHoverIcon = Utility::convertOpacity(m_connectedIcon.pixmap(iconSize), 0.2);

    // ui 初始化
    setPasswordEditorVisible(false);

    m_securityType = WirelessManagerInstance.networkBestSecurityType(m_deviceUni, m_ssid);

    auto device = NetworkManager::findNetworkInterface(deviceUni);
    auto wirelessDevice = device.objectCast<NetworkManager::WirelessDevice>();
    auto wirelessNetwork = wirelessDevice->findNetwork(ssid);
    // wifi信号变化
    connect(wirelessNetwork.data(), &NetworkManager::WirelessNetwork::signalStrengthChanged, this, &WirelessConnectionWidget::signalStrengthChanged);
    signalStrengthChanged(wirelessNetwork->signalStrength());

    m_status = NetStatus::DISCONNECTED;
    updateShowStatus();

    m_ui->labelName->setShowText(m_ssid);
}

WirelessConnectionWidget::~WirelessConnectionWidget()
{
    delete m_ui;
}

void WirelessConnectionWidget::updateStatus()
{
    auto device = NetworkManager::findNetworkInterface(m_deviceUni);
    NetworkManager::ActiveConnection::State state = NetworkManager::ActiveConnection::Deactivated;
    bool isLoading = false;

    auto activeConnection = device->activeConnection();
    if (activeConnection)
    {
        auto connectionSettings = activeConnection->connection()->settings();
        auto wifiSetting = connectionSettings->setting(NetworkManager::Setting::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
        if (wifiSetting->ssid() == m_ssid)
        {
            state = activeConnection->state();
            isLoading = device->state() > NetworkManager::Device::State::Disconnected && device->state() < NetworkManager::Device::State::Activated;
        }
    }

    NetStatus connectedFlag = m_status;
    setActiveStatus(state, isLoading);

    if (!m_firstUpdateFlag && connectedFlag != m_status)
    {
        // 连接状态变化通知
        if (m_status == CONNECTED)
        {
            Common::generalNotify(tr("wireless network"), m_ssid + " " + tr("connected"));
        }
        else if (m_status == DISCONNECTED)
        {
            Common::generalNotify(tr("wireless network"), m_ssid + " " + tr("disconnected"));
        }
    }
    m_firstUpdateFlag = false;
}

void WirelessConnectionWidget::requestPassword()
{
    bool isOK = false;
    QString title = tr("please input password");
    QString label = tr("WI-FI(%1) requires password re-entry").arg(m_ssid);

    QInputDialog dialog;
    dialog.setWindowTitle(title);
    dialog.setLabelText(label);
    dialog.setOkButtonText(tr("OK"));
    dialog.setCancelButtonText(tr("Cancel"));
    dialog.setTextEchoMode(QLineEdit::PasswordEchoOnEdit);

    QString passwd;
    if (dialog.exec() == QDialog::Accepted)
    {
        passwd = dialog.textValue();
        isOK = true;
    }
    else
    {
        isOK = false;
    }
    while (isOK && passwd.length() < 8)
    {
        dialog.setLabelText(label + "\n" + tr("The password must be at least 8 characters long."));
        if (dialog.exec() == QDialog::Accepted)
        {
            passwd = dialog.textValue();
            isOK = true;
        }
        else
        {
            isOK = false;
        }
    }

    // 被动输入密码连接
    emit respondPasswdRequest(m_ssid, passwd, !isOK);
}

void WirelessConnectionWidget::mouseReleaseEvent(QMouseEvent *event)
{
    // 密码输入框已显示，则隐藏
    if (m_ui->widgetPassword->isVisible())
    {
        setPasswordEditorVisible(false);
    }

    if (NetStatus::CONNECTED == m_status)
    {
        // on_toolButtonDisconnect_clicked();
        return;
    }
    else if (NetStatus::DISCONNECTED == m_status)
    {
        bool canDirectConn = WirelessManagerInstance.checkNetworkCanDirectConn(m_deviceUni, m_ssid);
        KLOG_INFO(LCSettingbar) << m_deviceUni << m_ssid << "checkNetworkCanDirectConn" << canDirectConn;
        if (canDirectConn)
        {
            WirelessManagerInstance.activateNetowrk(m_deviceUni, m_ssid);
            return;
        }

        if (SECURITY_TYPE_NONE == m_securityType)
        {
            emit addAndActivateNetwork(m_deviceUni, m_ssid, "");
        }
        else if (SECURITY_TYPE_WPA_AND_WPA2_PERSON == m_securityType || SECURITY_TYPE_WPA3_PERSON == m_securityType)
        {
            // 让用户输入密码
            setPasswordEditorVisible(true);
        }
        else
        {
            KLOG_WARNING(LCSettingbar) << "security type can not Support" << m_deviceUni << m_ssid << "securityType" << m_securityType;
            QString errorMessage = tr("security type can not Support");
            setToolTip(errorMessage);
            QToolTip::showText(m_ui->labelName->mapToGlobal({0, 0}), errorMessage, this);
            return;
        }
    }
}

void WirelessConnectionWidget::enterEvent(QEvent *event)
{
    //    KLOG_INFO() << "WirelessConnectionWidget::enterEvent";

    updateShowStatus();
}

void WirelessConnectionWidget::leaveEvent(QEvent *event)
{
    //    KLOG_INFO() << "WirelessConnectionWidget::leaveEvent";

    updateShowStatus();
}

void WirelessConnectionWidget::on_btnOkPassword_clicked()
{
    // 密码输入框已显示，则获取输入的密码
    QString password = m_ui->lineEditPassword->text();

    if (password.length() < 8)
    {
        QString errorMessage = tr("The password must be at least 8 characters long.");
        m_ui->lineEditPassword->setToolTip(errorMessage);
        QToolTip::showText(m_ui->labelName->mapToGlobal({0, 0}), errorMessage, m_ui->lineEditPassword);
        return;
    }

    // 主动输入密码连接
    emit addAndActivateNetwork(m_deviceUni, m_ssid, password);

    // 隐藏密码框
    on_btnCancelPassword_clicked();
}

void WirelessConnectionWidget::on_btnCancelPassword_clicked()
{
    setPasswordEditorVisible(false);
}

void WirelessConnectionWidget::setPasswordEditorVisible(bool isVisible)
{
    m_ui->widgetPassword->setVisible(isVisible);
    emit resizeShow();
}

void WirelessConnectionWidget::setActiveStatus(NetworkManager::ActiveConnection::State state, bool isLoading)
{
    KLOG_INFO(LCSettingbar) << "set active ui status:" << m_ssid << state;
    if (isLoading)
    {
        // 载入状态
        m_status = NetStatus::LOADING;
        updateShowStatus();
    }
    else
    {
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
        default:  // Deactivated Unknown
            m_status = NetStatus::DISCONNECTED;
            break;
        }
        updateShowStatus();
    }
}

void WirelessConnectionWidget::signalStrengthChanged(int strength)
{
    QString themeIcon = KS_ICON_WIRELESS_PREFIX;
    themeIcon += "-";
    if (SECURITY_TYPE_NONE != m_securityType)
    {
        themeIcon += KS_ICON_WIRELESS_SECURITY;
    }
    themeIcon += "-";
    if (0 <= strength && strength < 25)
        themeIcon += "0";
    else if (25 <= strength && strength < 50)
        themeIcon += "1";
    else if (50 <= strength && strength < 75)
        themeIcon += "2";
    else if (75 <= strength && strength <= 100)
        themeIcon += "3";

    KLOG_INFO(LCSettingbar) << "wireless signal strength changed" << m_ssid << strength;

    m_ui->toolButtonSignalStrength->setIcon(QIcon::fromTheme(themeIcon));
}

void WirelessConnectionWidget::updateShowStatus()
{
    //    KLOG_INFO() << "WirelessConnectionWidget::updateShowStatus" << m_status;
    switch (m_status)
    {
    case NetStatus::LOADING:
        m_loadingLabel->setVisible(true);
        m_connectStatu->setVisible(false);
        m_ui->layoutNetStatu->removeWidget(m_connectStatu);
        m_ui->layoutNetStatu->addWidget(m_loadingLabel);
        m_ui->toolButtonDisconnect->setVisible(false);
        break;
    case NetStatus::CONNECTED:
        m_loadingLabel->setVisible(false);
        m_connectStatu->setIcon(m_connectedIcon);
        m_connectStatu->setVisible(true);
        m_ui->layoutNetStatu->removeWidget(m_loadingLabel);
        m_ui->layoutNetStatu->addWidget(m_connectStatu);
        m_ui->toolButtonDisconnect->setVisible(true);
        break;
    default:
        m_ui->toolButtonDisconnect->setVisible(false);
        if (underMouse())
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

void WirelessConnectionWidget::on_toolButtonDisconnect_clicked()
{
    auto connection = NetCommon::getAvailableConnectionBySsid(m_deviceUni, m_ssid);
    if (!connection.isNull())
    {
        NetCommon::deactivateConnection(connection->uuid());
    }
}

}  // namespace SettingBar
}  // namespace Kiran
