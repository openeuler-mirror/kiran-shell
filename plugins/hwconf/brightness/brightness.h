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

#include <QObject>

class QDBusInterface;
class QDBusMessage;
class QDBusPendingCallWatcher;
namespace Kiran
{
namespace HwConf
{
class Brightness : public QObject
{
    Q_OBJECT
public:
    explicit Brightness(QObject* parent = nullptr);

    void init();
    void setBrightness(int value);   // 设置亮度
    bool getBrightness(int& value);  // 获取亮度

private slots:
    void updateBrightness();  // 亮度更新
    void dbusBrightnessChanged(QDBusMessage message);
    void dbusActiveProfileChanged(QDBusMessage message);

private:
    void handleSetBrightnessResult(QDBusPendingCallWatcher* watcher);  // 亮度设置dbus回传信息

signals:
    void enableBrightness(bool enabled);     // 禁用亮度
    void brightnessValueChanged(int value);  // 亮度变化
    void isReadyToUpdate(bool& flag);        // 获取界面是否已准备好接受变化：亮度变化来源未知，需要在界面调整时禁用外部变化同步到界面

private:
    QDBusInterface* m_interface;  // dbus接口
};
}  // namespace HwConf
}  // namespace Kiran
