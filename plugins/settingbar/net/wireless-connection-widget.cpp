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
      m_isConnected(false)
{
    m_ui->setupUi(this);

    // ui 初始化
    setPasswordEditorVisible(false);
    m_ui->labelName->setText(ssid);
    m_ui->labelInfo->clear();

    m_securityType = WirelessManagerInstance.networkBestSecurityType(m_deviceUni, m_ssid);

    auto device = NetworkManager::findNetworkInterface(deviceUni);
    auto wirelessDevice = device.objectCast<NetworkManager::WirelessDevice>();
    auto wirelessNetwork = wirelessDevice->findNetwork(ssid);
    // wifi信号变化
    connect(wirelessNetwork.data(), &NetworkManager::WirelessNetwork::signalStrengthChanged, this, &WirelessConnectionWidget::signalStrengthChanged);
    signalStrengthChanged(wirelessNetwork->signalStrength());

    // 状态重置
    resetStatus();
}

WirelessConnectionWidget::~WirelessConnectionWidget()
{
    delete m_ui;
}

void WirelessConnectionWidget::updateStatus()
{
    auto device = NetworkManager::findNetworkInterface(m_deviceUni);
    NetworkManager::WirelessDevice::Ptr wirelessDevice = device.objectCast<NetworkManager::WirelessDevice>();
    auto state = wirelessDevice->state();

    auto activeConnection = wirelessDevice->activeConnection();
    if (!activeConnection)
    {
        resetStatus();
        return;
    }

    auto connectionSettings = activeConnection->connection()->settings();
    auto wifiSetting = connectionSettings->setting(NetworkManager::Setting::Wireless).dynamicCast<NetworkManager::WirelessSetting>();

    if (wifiSetting->ssid() != m_ssid)
    {
        // 这个ssid不是激活的
        // 重置状态
        resetStatus();
        return;
    }

    KLOG_INFO(LCSettingbar) << "wireless device state:" << state;
    KLOG_INFO(LCSettingbar) << "wireless device activing:" << wifiSetting->ssid();
    KLOG_INFO(LCSettingbar) << "WirelessConnectionWidget::updateStatus" << state << activeConnection << activeConnection->state();

    setActiveStatus(activeConnection->state());
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

void WirelessConnectionWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    // 密码输入框已显示，则隐藏
    if (m_ui->widgetPassword->isVisible())
    {
        setPasswordEditorVisible(false);
    }

    if (m_isConnected)
    {
        auto connection = NetCommon::getAvailableConnectionBySsid(m_deviceUni, m_ssid);
        if (!connection.isNull())
        {
            NetCommon::deactivateConnection(connection->uuid());
        }
        return;
    }
    else
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

void WirelessConnectionWidget::setActiveStatus(NetworkManager::ActiveConnection::State state)
{
    KLOG_INFO(LCSettingbar) << "set active ui status:" << m_ssid << state;

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
}

void WirelessConnectionWidget::resetStatus()
{
    m_ui->labelLoading->setVisible(false);
    m_ui->toolButtonConnectStatu->setVisible(false);
    m_isConnected = false;
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

    //    KLOG_INFO(LCSettingbar) << "wireless signal strength changed" << m_ssid << strength;

    m_ui->toolButtonSignalStrength->setIcon(QIcon::fromTheme(themeIcon));
}

}  // namespace SettingBar
}  // namespace Kiran
