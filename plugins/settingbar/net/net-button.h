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

#include <NetworkManagerQt/ActiveConnection>
#include <NetworkManagerQt/Manager>
#include <QWidget>

#include "setting-button.h"

// 托盘网络按钮

namespace Kiran
{
namespace SettingBar
{
class NetButton : public SettingButton
{
    Q_OBJECT

public:
    NetButton(QWidget* parent = nullptr);

private:
    void updateNetworkStatus();

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
};
}  // namespace SettingBar
}  // namespace Kiran
