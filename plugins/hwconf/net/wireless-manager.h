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

#include <network-demo/wireless-network-manager.h>
#include <QObject>

// 无线管理类

namespace Kiran
{
namespace HwConf
{
#define WirelessManagerInstance WirelessManager::getInstance()

class WirelessManager : public QObject
{
    Q_OBJECT
public:
    static WirelessManager& getInstance();

    // 无线管理
    void AddToManager(const QString& deviceUni);
    void RemoveFromManager(const QString& deviceUni);

    // 无线信息获取
    WirelessNetworkInfoList getNetworkInfoList(const QString& deviceUni);

    // 无线连接
    void addAndActivateConnection(const QString& deviceUni, const QString& ssid, const QString& password);

private:
    explicit WirelessManager(QObject* parent = nullptr);

signals:
    // 无线连接点变化
    void networkAppeared(QString deviceUni, QString ssid);
    void networkDisappeared(QString deviceUni, QString ssid);

private:
    QMap<QString, WirelessNetworkManager*> m_deviceManagerMap;
};
}  // namespace HwConf
}  // namespace Kiran
