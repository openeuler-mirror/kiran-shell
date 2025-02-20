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

#include <NetworkManagerQt/Connection>
#include <NetworkManagerQt/Device>
#include <QWidget>

namespace Ui
{
class WiredConnectionWidget;
}

// 连接树中的有线连接项
// 使用设备uuid和连接uuid标识

namespace Kiran
{
namespace SettingBar
{
class WiredConnectionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WiredConnectionWidget(QString deviceUni, QString connectionUuid, QWidget *parent = nullptr);
    ~WiredConnectionWidget() override;

    void updateStatus();

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    Ui::WiredConnectionWidget *m_ui;

    // 固定属性
    QString m_deviceUni;
    QString m_connectionUuid;

    // 变动属性
    bool m_isConnected;
};
}  // namespace SettingBar
}  // namespace Kiran
