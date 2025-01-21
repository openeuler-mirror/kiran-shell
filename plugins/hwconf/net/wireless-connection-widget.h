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

#pragma once

#include <kiran-desktop/network-common.h>
#include <QWidget>

#include "net-common.h"

namespace Ui
{
class WirelessConnectionWidget;
}

// 连接树中的无线连接项
// 使用设备uuid和连接ssid标识

namespace Kiran
{
namespace HwConf
{
class WirelessConnectionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WirelessConnectionWidget(QString deviceUni, QString ssid, QWidget *parent = nullptr);
    ~WirelessConnectionWidget();

    void updateStatus();

    void requestPassword();

protected:
    void mouseDoubleClickEvent(QMouseEvent *event);

private slots:
    void on_btnOkPassword_clicked();
    void on_btnCancelPassword_clicked();

private:
    void setPasswordEditorVisible(bool isVisible);

    void setActiveStatus(NetworkManager::ActiveConnection::State state);
    void resetStatus();
    void signalStrengthChanged(int strength);

signals:
    void addAndActivateNetwork(QString deviceUni, QString ssid, const QString password);
    void respondPasswdRequest(QString ssid, QString password, bool isCancel);

    void resizeShow();

private:
    Ui::WirelessConnectionWidget *ui;

    // 固定属性
    QString m_deviceUni;
    QString m_ssid;
    WifiSecurityType m_securityType;

    // 变动属性
    bool m_isConnected;
};
}  // namespace HwConf
}  // namespace Kiran
