/**
 * Copyright (c) 2020 ~ 2025 KylinSec Co., Ltd.
 * kiran-shell is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     liuxinhao <liuxinhao@kylinsec.com.cn>
 */

#pragma once

#include <kiran-input-dialog.h>

namespace Kiran
{
namespace SettingBar
{
class WirelessDialog : public KiranInputDialog
{
    Q_OBJECT
    enum DialogType
    {
        WIRELESS_DIALOG_TYPE_PASSWD,
        WIRELESS_DIALOG_TYPE_HIDDEN_SSID,
    };

public:
    ~WirelessDialog();

    static bool getNetworkPasswd(QWidget* w, const QString& title, const QString& ssid, QString& passwd);
    static bool getHiddenNetworkSsid(QWidget* w, QString& ssid);

private:
    WirelessDialog(DialogType type, QWidget* parent = nullptr);
    void init();
    bool checkValid(const QString& text) override;

private:
    DialogType m_type;
};
}  // namespace SettingBar
}  // namespace Kiran
