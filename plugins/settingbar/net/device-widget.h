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

#include <NetworkManagerQt/Device>
#include <QWidget>

namespace Ui
{
class DeviceWidget;
}

// 连接树中的设备项，分为有线和无线
// 使用设备uuid标识

namespace Kiran
{
namespace SettingBar
{
class DeviceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceWidget(NetworkManager::Device::Type type, QString deviceUni, QWidget *parent = nullptr);
    ~DeviceWidget() override;

    void Init();

private:
    void updateInfo();

signals:
    void updateDeviceStatu();

private slots:
    void on_toolButtonScan_clicked();

private:
    Ui::DeviceWidget *m_ui;

    NetworkManager::Device::Type m_deviceType;
    QString m_deviceUni;

    QTimer *m_wifiScanTimer;
};
}  // namespace SettingBar
}  // namespace Kiran
