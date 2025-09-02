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
#include <QIcon>
#include <QWidget>

#include "net-common.h"

namespace Ui
{
class WirelessConnectionWidget;
}

class StyledButton;
class LoadingLabel;

// 连接树中的无线连接项
// 使用设备uuid和连接ssid标识

namespace Kiran
{
namespace SettingBar
{
class WirelessConnectionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WirelessConnectionWidget(QString deviceUni, QString ssid, QWidget *parent = nullptr);
    ~WirelessConnectionWidget() override;

    void updateStatus();

    void requestPassword();

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

private slots:
    void on_btnOkPassword_clicked();
    void on_btnCancelPassword_clicked();
    void on_toolButtonDisconnect_clicked();

private:
    void setPasswordEditorVisible(bool isVisible);
    void signalStrengthChanged(int strength);
    void updateShowStatus();

signals:
    void addAndActivateNetwork(QString deviceUni, QString ssid, QString password);
    void respondPasswdRequest(QString ssid, QString password, bool isCancel);

    void resizeShow();

private:
    Ui::WirelessConnectionWidget *m_ui;

    StyledButton *m_connectStatu;
    LoadingLabel *m_loadingLabel;

    // 固定属性
    QString m_deviceUni;
    QString m_ssid;
    WifiSecurityType m_securityType;

    NetShowState m_status;

    // 第一次启动不需要通知
    bool m_firstUpdateFlag = true;

    QIcon m_connectedIcon;
    QIcon m_connectedHoverIcon;
};
}  // namespace SettingBar
}  // namespace Kiran
