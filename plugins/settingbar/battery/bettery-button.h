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

#include "setting-button.h"

class QGSettings;
class QDBusMessage;
class DBusUPowerService;
class DBusUPowerDevice;
namespace Kiran
{
namespace SettingBar
{
class BatteryButton : public SettingButton
{
    Q_OBJECT
public:
    BatteryButton(QWidget* parent = nullptr);

    void init();

private:
    void serviceOwnerChanged(const QString& serviceName, const QString& oldOwner, const QString& newOwner);

    void updateIcon();           // 更新显示图标
    QString getIconName();       // 获取显示图标
    void updateDisplayDevice();  // 电池显示设备更新

    void disableBattery();

    QString percent2IconIndex(uint percentage);  // 电量百分比转图标序号

private slots:
    void uPowerdDusPropertiesChanged(QDBusMessage message);  // 电池管理属性变化

signals:
    void enableBattery(bool enabled);         // 禁用电池
    void batteryValueChanged(QString value);  // 电量变化
    void batteryIconChanged(QIcon icon);      // 电量图标变化

private:
    DBusUPowerService* m_upower;       // 电池管理接口
    DBusUPowerDevice* m_upowerDevice;  // 电池设备接口
    QGSettings* m_gsettings;           // 电池配置（图标显示策略）
};
}  // namespace SettingBar
}  // namespace Kiran
