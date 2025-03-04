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

#include <QToolButton>

#include "net-common.h"
#include "setting-item.h"

// 配置页中的网络项，分为有线和无线
// 界面上存在一个图标按钮，一个前进按钮（触发显示二级页面（连接树））

namespace Kiran
{
namespace SettingBar
{
class netTreeWidget;
class NetConfItem : public SettingItem
{
    Q_OBJECT
public:
    NetConfItem(NetworkManager::Device::Type type, QWidget* parent = nullptr);
    ~NetConfItem() override;

    void init();

private:
    void initUI();
    void updateNetworkStatus();

    void netIconClicked();
    void netSettingClicked();

signals:
    void enableNetwork(bool enabled);

private:
    // 网络类型
    NetworkManager::Device::Type m_netType;

    // 二级设置页面
    QWidget* m_settingWidget = nullptr;
    QToolButton* m_backBtn = nullptr;
    netTreeWidget* m_connectTreeWidget = nullptr;
};
}  // namespace SettingBar
}  // namespace Kiran
