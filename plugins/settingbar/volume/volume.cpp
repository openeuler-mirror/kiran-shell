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
#include <QDBusArgument>
#include <QDBusMessage>
#include <QJsonDocument>
#include <QJsonParseError>
#include <thread>

#include "ks_audio_device_interface.h"
#include "ks_audio_interface.h"
#include "lib/common/dbus-service-watcher.h"
#include "lib/common/logging-category.h"
#include "lib/common/utility.h"
#include "volume.h"

#define AUDIO_DBUS_SERVICE "com.kylinsec.Kiran.SessionDaemon.Audio"
#define AUDIO_OBJECT_PATH "/com/kylinsec/Kiran/SessionDaemon/Audio"
#define PROPERTIES_INTERFACE "org.freedesktop.DBus.Properties"
#define PROPERTIES_CHANGED "PropertiesChanged"

namespace Kiran
{
namespace SettingBar
{
Volume::Volume(QObject *parent)
    : QObject{parent},
      m_ksAudio(nullptr),
      m_ksAudioDevice(nullptr)
{
    connect(this, &Volume::readyToInitDefaultSink, this, &Volume::initDefaultSink);

    connect(&DBusWatcher, &DBusServiceWatcher::serviceOwnerChanged, this, &Volume::serviceOwnerChanged);
    DBusWatcher.AddService(AUDIO_DBUS_SERVICE, QDBusConnection::SessionBus);
}

void Volume::serviceOwnerChanged(const QString &service, const QString &oldOwner, const QString &newOwner)
{
    if (AUDIO_DBUS_SERVICE != service)
    {
        return;
    }
    if (oldOwner.isEmpty())
    {
        KLOG_INFO(LCSettingbar) << "dbus service registered:" << service;
        init();
    }
    else if (newOwner.isEmpty())
    {
        KLOG_INFO(LCSettingbar) << "dbus service unregistered:" << service;
        emit enableVolume(false);
    }
}

void Volume::setVolume(const int &value)
{
    if (!m_ksAudioDevice)
    {
        return;
    }

    double volumeValue = value / 100.0;
    //    m_ksAudioDevice->call("SetVolume", volumeValue);
    m_ksAudioDevice->SetVolume(volumeValue);
}

void Volume::setMute(const bool &isMute)
{
    if (!m_ksAudioDevice)
    {
        return;
    }

    m_ksAudioDevice->SetMute(isMute);
    KLOG_INFO(LCSettingbar) << "current defalut sink mute:"
                            << m_ksAudioDevice->mute();
}

bool Volume::getVolume(int &value)
{
    if (!m_ksAudioDevice)
    {
        return false;
    }

    auto volume = m_ksAudioDevice->volume();
    double currentVolumeDouble = volume * 100;
    value = round(currentVolumeDouble);
    return true;
}

bool Volume::getMute(bool &isMute)
{
    if (!m_ksAudioDevice)
    {
        return false;
    }

    isMute = m_ksAudioDevice->mute();
    return true;
}

void Volume::sinkPropertiesChanged(QDBusMessage message)
{
    QList<QVariant> args = message.arguments();
    if (args.count() < 2)
    {
        return;
    }

    QString interfaceName = args.at(0).toString();
    QVariantMap changedProperties =
        qdbus_cast<QVariantMap>(args.at(1).value<QDBusArgument>());
    for (auto iter = changedProperties.begin(); iter != changedProperties.end();
         iter++)
    {
        QString property = iter.key();
        QVariant propertyValue = iter.value();

        if (QStringLiteral("volume") == property)
        {
            double currentVolumeDouble = propertyValue.toDouble() * 100;
            int currentVolume = round(currentVolumeDouble);
            emit volumeValueChanged(currentVolume);
        }
        else if (QStringLiteral("mute") == property)
        {
            emit volumeMuteChanged(propertyValue.toBool());
        }
        else if (QStringLiteral("active_port") == property)
        {
            sinkActivePortChanged(propertyValue.toString());
        }
    }
}

void Volume::sinkActivePortChanged(const QString &value)
{
    if (!m_ksAudioDevice)
    {
        return;
    }

    KLOG_INFO(LCSettingbar) << "active port changed" << value;
    if (value.isEmpty())
    {
        emit enableVolume(false);
        return;
    }

    int currentVolume = 0;
    getVolume(currentVolume);
    emit volumeValueChanged(currentVolume);
}

void Volume::defaultSinkUpdate()
{
    KLOG_INFO(LCSettingbar) << "default audio sink update";
    // delete and restart init defaultSink
    if (m_ksAudioDevice != nullptr)
    {
        m_ksAudioDevice->deleteLater();
        m_ksAudioDevice = nullptr;
    }

    auto t = std::thread(&Volume::getDefaultSinkPath, this);
    t.detach();
}

void Volume::getDefaultSinkPath()
{
    m_defaultSinkPath.clear();
    int retryTimes = 3;
    while (retryTimes--)
    {
        auto reply = m_ksAudio->GetDefaultSink();
        if (reply.isError() || reply.value().isEmpty())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            continue;
        }
        m_defaultSinkPath = reply.value();
        if (m_defaultSinkPath.isEmpty())
        {
            KLOG_ERROR(LCSettingbar) << "GetDefaultSink failed:" << reply;
        }
    }

    if (m_defaultSinkPath.isEmpty())
    {
        emit enableVolume(false);
        return;
    }

    KLOG_INFO(LCSettingbar) << "default audio sink:" << m_defaultSinkPath;
    // 线程中不能创建QDBusInterface对象，需要发信号到主线程，由主线程创建
    emit readyToInitDefaultSink();
}

void Volume::initDefaultSink()
{
    m_ksAudioDevice = new KSAudioDevice(AUDIO_DBUS_SERVICE, m_defaultSinkPath,
                                        QDBusConnection::sessionBus(), this);
    QDBusConnection::sessionBus().connect(
        AUDIO_DBUS_SERVICE, m_defaultSinkPath, PROPERTIES_INTERFACE,
        PROPERTIES_CHANGED, this,
        SLOT(sinkPropertiesChanged(QDBusMessage)));

    initAudioDevice();
}

void Volume::sinkAdded()
{
    if (m_ksAudioDevice != nullptr)
    {
        // defaultSink不存在，则重新初始化设备
        initAudioDevice();
    }
}

void Volume::sinkDelete(uint index)
{
    if (m_ksAudioDevice != nullptr)
    {
        // 删除的是defaultSink
        if (m_ksAudioDevice->index() == index)
        {
            emit enableVolume(false);
        }
    }
}

void Volume::init()
{
    if (m_ksAudio)
    {
        m_ksAudio->deleteLater();
        m_ksAudio = nullptr;
    }

    m_ksAudio = new KSAudio(AUDIO_DBUS_SERVICE, AUDIO_OBJECT_PATH,
                            QDBusConnection::sessionBus(), this);
    if (!m_ksAudio->isValid())
    {
        emit enableVolume(false);
        return;
    }

    emit enableVolume(true);

    connect(m_ksAudio, SIGNAL(SinkAdded(uint)), this,
            SLOT(sinkAdded()));
    connect(m_ksAudio, SIGNAL(SinkDelete(uint)), this,
            SLOT(sinkDelete(uint)));
    connect(m_ksAudio, SIGNAL(DefaultSinkChange(uint)), this,
            SLOT(defaultSinkUpdate()));

    defaultSinkUpdate();
}

void Volume::initAudioDevice()
{
    if (!m_ksAudioDevice)
    {
        return;
    }

    auto getPorts = m_ksAudioDevice->GetPorts();
    KLOG_INFO(LCSettingbar) << "current audio device ports:" << getPorts;
    // 解析默认sink的端口信息
    QJsonParseError jsonParseError;
    QJsonDocument doc = QJsonDocument::fromJson(
        getPorts.value().toUtf8(), &jsonParseError);
    if (!doc.isNull() && jsonParseError.error == QJsonParseError::NoError)
    {
        int currentVolume = 0;
        getVolume(currentVolume);
        emit volumeValueChanged(currentVolume);
        emit enableVolume(true);
        return;
    }
    else
    {
        // 无激活端口则禁用音量设置
        emit enableVolume(false);
        return;
    }
}

}  // namespace SettingBar
}  // namespace Kiran
