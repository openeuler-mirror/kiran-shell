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
#include <QIcon>
#include <QWidget>

#include "net-common.h"

namespace Ui
{
class WiredConnectionWidget;
}

class StyledButton;
class LoadingLabel;

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
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

private slots:
    void on_toolButtonDisconnect_clicked();

private:
    void setActiveStatus(NetworkManager::ActiveConnection::State state);
    void updateShowStatus();

private:
    Ui::WiredConnectionWidget *m_ui;

    StyledButton *m_connectStatu;
    LoadingLabel *m_loadingLabel;

    // 固定属性
    QString m_deviceUni;
    QString m_connectionUuid;

    NetShowState m_status;

    // 第一次启动不需要通知
    bool m_firstUpdateFlag;

    QIcon m_connectedIcon;
    QIcon m_connectedHoverIcon;
};
}  // namespace SettingBar
}  // namespace Kiran
