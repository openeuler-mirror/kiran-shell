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

#include <QWidget>

#include "hw-conf-button.h"

namespace Kiran
{
namespace HwConf
{
class Volume;
class VolumeButton : public HwConfButton
{
    Q_OBJECT
public:
    VolumeButton(QWidget* parent = nullptr);

    void updateVolume();               // 主动更新音量
    void setVolume(const int& value);  // 设置音量
    void setVolumeMute();              // 设置是否静音变化

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    void setVolumeIcon(int volume);

signals:
    void enableVolume(bool enabled);
    void volumeValueChanged(int currentVolume);
    void volumeIconChanged(QIcon icon);

private:
    Volume* m_volume;
};
}  // namespace HwConf
}  // namespace Kiran
