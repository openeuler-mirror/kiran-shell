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
#include <NetworkManagerQt/ActiveConnection>
#include <NetworkManagerQt/Device>
#include <QObject>

// 无线管理类

namespace Kiran
{
class WirelessNetworkManager;
class NMSecretAgent;
namespace SettingBar
{
#define WirelessManagerInstance WirelessManager::getInstance()

class WirelessManager : public QObject
{
    Q_OBJECT
public:
    static WirelessManager& getInstance();

    // 获取当前管理的设备
    QStringList getDevices();

    // 无线信息获取
    WirelessNetworkInfoList getNetworkInfoList(const QString& deviceUni);
    WifiSecurityType networkBestSecurityType(const QString& deviceUni, const QString& ssid);

    // 无线连接判断
    bool checkNetworkCanDirectConn(const QString& deviceUni, const QString& ssid);
    // 无线连接
    void activateNetowrk(const QString& deviceUni, const QString& ssid);
    void addAndActivateNetwork(const QString& deviceUni, const QString& ssid, const QString& password);
    void respondPasswdRequest(const QString& ssid, const QString& password, bool isCancel);

    void scan(const QString& deviceUni);

private:
    explicit WirelessManager(QObject* parent = nullptr);

    void changeActiveConnection();

    // 更新设备管理列表
    void updateNetworkStatus();

    // 无线管理
    void AddToManager(const QString& deviceUni);
    void RemoveFromManager(const QString& deviceUni);

signals:
    void netStatusChanged();

    // 无线连接点变化
    void networkAppeared(QString deviceUni, QString ssid);
    void networkDisappeared(QString deviceUni, QString ssid);

    // 设备状态
    void stateChanged(QString deviceUni, NetworkManager::Device::State state);
    void activeConnectionStateChanged(QString deviceUni, NetworkManager::ActiveConnection::State state);

    // SecretAgent请求密码
    void requestPassword(const QString& devicePath, const QString& ssid, bool wait);

private:
    QMap<QString, WirelessNetworkManager*> m_deviceManagerMap;
    NMSecretAgent* m_secretAgent = nullptr;
};
}  // namespace SettingBar
}  // namespace Kiran
