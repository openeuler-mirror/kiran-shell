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

#include <QDBusMessage>
#include <QWidget>

#include "hw-conf-button.h"

class QDBusInterface;
class QGSettings;
class QDBusMessage;
namespace Kiran
{
namespace HwConf
{
class BatteryButton : public HwConfButton
{
    Q_OBJECT
public:
    BatteryButton(QWidget* parent = nullptr);

    void updateBattery();

private:
    void settingChanged(const QString& key);  // 电池配置变化

    void updateIcon();           // 更新显示图标
    QString getIconName();       // 获取显示图标
    void updateDisplayDevice();  // 电池显示设备更新

    QString percent2IconIndex(uint percentage);  // 电量百分比转图标序号

private slots:
    void uPowerdDusPropertiesChanged(QDBusMessage message);  // 电池管理属性变化

signals:
    void enableBattery(bool enabled);         // 禁用电池
    void batteryValueChanged(QString value);  // 电量变化
    void batteryIconChanged(QIcon icon);      // 电量图标变化

private:
    QDBusInterface* m_interface;        // 电池管理接口
    QDBusInterface* m_interfaceDevice;  // 电池设备接口
    QGSettings* m_gsettings;            // 电池配置（图标显示策略）
};
}  // namespace HwConf
}  // namespace Kiran
