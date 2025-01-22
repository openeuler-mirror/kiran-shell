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
#include <QObject>

// 有线管理类

namespace Kiran
{
namespace HwConf
{
#define WiredManagerInstance WiredManager::getInstance()

class WiredManager : public QObject
{
    Q_OBJECT
public:
    static WiredManager& getInstance();

    // 有线管理
    void AddToManager(const QString& deviceUni);
    void RemoveFromManager(const QString& deviceUni);

private:
    explicit WiredManager(QObject* parent = nullptr);

signals:
    // 有线连接点变化
    void availableConnectionAppeared(QString deviceUni, QString connectionUuid);
    void availableConnectionDisappeared(QString deviceUni, QString connectionUuid);

    // 设备状态
    void stateChanged(QString deviceUni, NetworkManager::Device::State state);
    void activeConnectionChanged();

private:
    QStringList m_deviceUnis;
};
}  // namespace HwConf
}  // namespace Kiran
