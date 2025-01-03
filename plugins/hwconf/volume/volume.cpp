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
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusServiceWatcher>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonParseError>
#include <cmath>

#include "volume.h"

#define AUDIO_DBUS_SERVICE "com.kylinsec.Kiran.SessionDaemon.Audio"
#define AUDIO_OBJECT_PATH "/com/kylinsec/Kiran/SessionDaemon/Audio"
#define AUDIO_DBUS_INTERFACE_NAME "com.kylinsec.Kiran.SessionDaemon.Audio"

#define AUDIO_SINK_OBJECT_PATH "/com/kylinsec/Kiran/SessionDaemon/Audio/Sink"
#define AUDIO_SOURCE_OBJECT_PATH "/com/kylinsec/Kiran/SessionDaemon/Audio/Source"
#define AUDIO_DEVICE_DBUS_INTERFACE_NAME "com.kylinsec.Kiran.SessionDaemon.Audio.Device"

#define PROPERTIES_INTERFACE "org.freedesktop.DBus.Properties"
#define PROPERTIES_CHANGED "PropertiesChanged"

namespace Kiran
{
namespace HwConf
{
Volume::Volume(QObject *parent)
    : QObject{parent}
{
    m_audioInterface = new QDBusInterface(AUDIO_DBUS_SERVICE, AUDIO_OBJECT_PATH,
                                          AUDIO_DBUS_INTERFACE_NAME,
                                          QDBusConnection::sessionBus(), this);

    QDBusMessage msg = m_audioInterface->call("GetDefaultSink");
    KLOG_DEBUG() << "GetDefaultSink:" << msg.arguments().first().toString();
    QString defaultSinkPath = msg.arguments().first().toString();
    m_defaultSink = new QDBusInterface(AUDIO_DBUS_SERVICE, defaultSinkPath,
                                AUDIO_DEVICE_DBUS_INTERFACE_NAME,
                                QDBusConnection::sessionBus(), this);

    initAudioDevice();

    QDBusConnection::sessionBus().connect(
        AUDIO_DBUS_SERVICE, defaultSinkPath, PROPERTIES_INTERFACE,
        PROPERTIES_CHANGED, this,
        SLOT(sinkPropertiesChanged(QDBusMessage)));

    connect(m_audioInterface, SIGNAL(SinkAdded(uint)), this,
            SLOT(sinkAdded(uint)));
    connect(m_audioInterface, SIGNAL(SinkDelete(uint)), this,
            SLOT(sinkDelete(uint)));
    connect(m_audioInterface, SIGNAL(DefaultSinkChange(uint)), this,
            SLOT(defaultSinkChanged(uint)));

    m_dbusServiceWatcher = new QDBusServiceWatcher(this);
    m_dbusServiceWatcher->setConnection(QDBusConnection::sessionBus());
    m_dbusServiceWatcher->addWatchedService(AUDIO_DBUS_SERVICE);
    m_dbusServiceWatcher->setWatchMode(QDBusServiceWatcher::WatchForOwnerChange);
    connect(m_dbusServiceWatcher, &QDBusServiceWatcher::serviceOwnerChanged,
            [this](const QString &service, const QString &oldOwner, const QString &newOwner)
            {
                // Note that this signal is also emitted whenever the serviceName service was registered or unregistered.
                // If it was registered, oldOwner will contain an empty string,
                // whereas if it was unregistered, newOwner will contain an empty string
                if (oldOwner.isEmpty())
                {
                    KLOG_INFO() << "dbus service registered:" << service;
                    emit enableVolume(true);
                }
                else if (newOwner.isEmpty())
                {
                    KLOG_INFO() << "dbus service unregistered:" << service;
                    emit enableVolume(false);
                }
            });
}

void Volume::setVolume(const int &value)
{
    double volumeValue = value / 100.0;
    m_defaultSink->call("SetVolume", volumeValue);
}

void Volume::setMute(const bool &isMute)
{
    m_defaultSink->call("SetMute", isMute);
    KLOG_DEBUG() << "current defalut sink mute:"
                 << m_defaultSink->property("mute").toBool();
}

bool Volume::getVolume(int &value)
{
    auto volume = m_defaultSink->property("volume");
    if (volume.isValid())
    {
        double currentVolumeDouble = volume.toDouble() * 100;
        value = round(currentVolumeDouble);
        return true;
    }

    return false;
}

bool Volume::getMute(bool &isMute)
{
    auto mute = m_defaultSink->property("mute");
    if (mute.isValid())
    {
        isMute = mute.toBool();
        return true;
    }

    return false;
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
    KLOG_INFO() << "active port changed :" << value;
    if (value.isEmpty())
    {
        emit enableVolume(false);
        return;
    }

    double currentVolumeDouble = m_defaultSink->property("volume").toDouble() * 100;
    int currentVolume = round(currentVolumeDouble);
    emit volumeValueChanged(currentVolume);
}

void Volume::defaultSinkChanged(uint index)
{
    KLOG_INFO() << "Default Sink Changed";
    // delete and restart init defaultSink
    if (m_defaultSink != nullptr)
    {
        m_defaultSink->deleteLater();
        m_defaultSink = nullptr;
    }

    QDBusMessage msg = m_audioInterface->call("GetDefaultSink");
    KLOG_DEBUG() << "GetDefaultSink:" << msg.arguments().first().toString();
    QString defaultSinkPath = msg.arguments().first().toString();
    m_defaultSink = new QDBusInterface(AUDIO_DBUS_SERVICE, defaultSinkPath,
                                AUDIO_DEVICE_DBUS_INTERFACE_NAME,
                                QDBusConnection::sessionBus(), this);
    initAudioDevice();
    QDBusConnection::sessionBus().connect(
        AUDIO_DBUS_SERVICE, defaultSinkPath, PROPERTIES_INTERFACE,
        PROPERTIES_CHANGED, this,
        SLOT(handleSinkPropertiesChanged(QDBusMessage)));
}

void Volume::sinkAdded(uint index)
{
    if (m_defaultSink != nullptr)
    {
        // defaultSink不存在，则重新初始化设备
        initAudioDevice();
    }
}

void Volume::sinkDelete(uint index)
{
    if (m_defaultSink != nullptr)
    {
        // 删除的是defaultSink
        if (m_defaultSink->property("index").toInt() == index)
        {
            emit enableVolume(false);
        }
    }
}

void Volume::initAudioDevice()
{
    auto getPorts = m_defaultSink->call("GetPorts");
    KLOG_DEBUG() << "getPorts" << getPorts;
    // 解析默认sink的端口信息
    QJsonParseError jsonParseError;
    QJsonDocument doc = QJsonDocument::fromJson(
        getPorts.arguments().first().toByteArray(), &jsonParseError);
    if (!doc.isNull() && jsonParseError.error == QJsonParseError::NoError)
    {
        double currentVolumeDouble = m_defaultSink->property("volume").toDouble() * 100;
        int currentVolume = round(currentVolumeDouble);
        emit volumeValueChanged(currentVolume);
        KLOG_DEBUG() << "currentVolume" << currentVolume;
    }
    else
    {
        // 无激活端口则禁用音量设置
        emit enableVolume(false);
    }
}
}  // namespace HwConf
}  // namespace Kiran
