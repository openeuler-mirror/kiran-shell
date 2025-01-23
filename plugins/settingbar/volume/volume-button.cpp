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

#include <qt5-log-i.h>
#include <QContextMenuEvent>
#include <QMenu>
#include <QProcess>

#include "ks-i.h"
#include "lib/common/logging-category.h"
#include "volume-button.h"
#include "volume/volume.h"

namespace Kiran
{
namespace SettingBar
{
VolumeButton::VolumeButton(QWidget *parent)
    : SettingButton(parent)
{
    m_volume = new Volume(this);
    connect(m_volume, &Volume::enableVolume, [this](bool enabled)
            {
                setVisible(enabled);
                emit enableVolume(enabled);
            });
    connect(m_volume, &Volume::volumeValueChanged, [this](int value)
            {
                setVolumeIcon(value);
                emit volumeValueChanged(value);
            });
    connect(m_volume, &Volume::volumeMuteChanged, [this](bool isMute)
            {
                if (isMute)
                {
                    setVolumeIcon(0);
                }
                else
                {
                    int value;
                    if (m_volume->getVolume(value))
                    {
                        setVolumeIcon(value);
                    }
                }
            });
}

void VolumeButton::init()
{
    m_volume->init();

    int curVolume;
    bool isMute;
    if (m_volume->getVolume(curVolume) && m_volume->getMute(isMute))
    {
        setVolumeIcon(curVolume);
        if (isMute)
        {
            setVolumeIcon(0);
        }

        emit volumeValueChanged(curVolume);
    }
}

void VolumeButton::setVolume(const int &value)
{
    m_volume->setVolume(value);
}

void VolumeButton::setVolumeMute()
{
    bool isMute;
    if (m_volume->getMute(isMute))
    {
        m_volume->setMute(!isMute);
    }
}

void VolumeButton::setVolumeIcon(int volume)
{
    QIcon icon;
    if (volume == 0)
    {
        icon = QIcon::fromTheme(KS_ICON_HWCONF_AUDIO_MUTE);
    }
    else if (0 < volume && volume <= 34)
    {
        icon = QIcon::fromTheme(KS_ICON_HWCONF_AUDIO_LOW);
    }
    else if (33 < volume && volume <= 67)
    {
        icon = QIcon::fromTheme(KS_ICON_HWCONF_AUDIO_MEDIUM);
    }
    else
    {
        icon = QIcon::fromTheme(KS_ICON_HWCONF_AUDIO_LOUD);
    }
    setIcon(icon);
    emit volumeIconChanged(icon);
}
void VolumeButton::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu;
    menu.addAction(tr("Audio Settings"), this, [this]()
                   {
                       KLOG_INFO(LCSettingbar) << "start detached: kiran-control-panel -c audio";
                       QProcess::startDetached("kiran-control-panel", {"-c", "audio"});
                   });

    menu.exec(mapToGlobal(event->pos()));
    update();
}
}  // namespace SettingBar
}  // namespace Kiran
