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

class QDBusMessage;
class KSAudio;
class KSAudioDevice;
namespace Kiran
{
namespace HwConf
{
class Volume : public QObject
{
    Q_OBJECT
public:
    explicit Volume(QObject *parent = nullptr);

    void init();

    void setVolume(const int &value);
    void setMute(const bool &isMute);
    bool getVolume(int &value);
    bool getMute(bool &isMute);

private slots:
    void sinkPropertiesChanged(QDBusMessage msg);
    void sinkActivePortChanged(const QString &value);

    void defaultSinkUpdate();
    void getDefaultSinkPath();
    void initDefaultSink();
    void sinkAdded();
    void sinkDelete(uint index);

private:
    void initAudioDevice();
    void serviceOwnerChanged(const QString &service, const QString &oldOwner, const QString &newOwner);

signals:
    void enableVolume(bool enabled);
    void volumeValueChanged(int currentVolume);
    void volumeMuteChanged(bool isMute);
    void readyToInitDefaultSink();

private:
    KSAudio *m_ksAudio;
    KSAudioDevice *m_ksAudioDevice;
    QString m_defaultSinkPath;
};
}  // namespace HwConf
}  // namespace Kiran
