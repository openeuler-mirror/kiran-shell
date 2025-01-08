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
#include <QJsonDocument>
#include <QJsonParseError>
#include <cmath>
#include <thread>

#include "lib/common/dbus-service-watcher.h"
#include "lib/common/utility.h"
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
    : QObject{parent},
      m_audioInterface(nullptr),
      m_defaultSink(nullptr)
{
    connect(this, &Volume::readyToInitDefaultSink, this, &Volume::initDefaultSink);

    connect(&DBusWatcher, &DBusServiceWatcher::serviceOwnerChanged,
            [this](const QString &service, const QString &oldOwner, const QString &newOwner)
            {
                if (AUDIO_DBUS_SERVICE != service)
                {
                    return;
                }
                if (oldOwner.isEmpty())
                {
                    KLOG_INFO() << "dbus service registered:" << service;
                    init();
                }
                else if (newOwner.isEmpty())
                {
                    KLOG_INFO() << "dbus service unregistered:" << service;
                    emit enableVolume(false);
                }
            });
    DBusWatcher.AddService(AUDIO_DBUS_SERVICE, QDBusConnection::SessionBus);
}

void Volume::setVolume(const int &value)
{
    if (!m_defaultSink)
    {
        return;
    }

    double volumeValue = value / 100.0;
    m_defaultSink->call("SetVolume", volumeValue);
}

void Volume::setMute(const bool &isMute)
{
    if (!m_defaultSink)
    {
        return;
    }

    m_defaultSink->call("SetMute", isMute);
    KLOG_DEBUG() << "current defalut sink mute:"
                 << m_defaultSink->property("mute").toBool();
}

bool Volume::getVolume(int &value)
{
    if (!m_defaultSink)
    {
        return false;
    }

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
    if (!m_defaultSink)
    {
        return false;
    }

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
    if (!m_defaultSink)
    {
        return;
    }

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

void Volume::defaultSinkUpdate()
{
    KLOG_INFO() << "default audio sink update";
    // delete and restart init defaultSink
    if (m_defaultSink != nullptr)
    {
        m_defaultSink->deleteLater();
        m_defaultSink = nullptr;
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
        QDBusMessage msg = m_audioInterface->call("GetDefaultSink");
        if (QDBusMessage::ErrorMessage == msg.type() || msg.arguments().size() < 1 || msg.arguments().first().toString().isEmpty())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            continue;
        }

        m_defaultSinkPath = msg.arguments().size() > 0 ? msg.arguments().first().toString() : "";
        if (m_defaultSinkPath.isEmpty())
        {
            KLOG_ERROR() << "GetDefaultSink failed:" << msg;
        }
    }

    if (m_defaultSinkPath.isEmpty())
    {
        emit enableVolume(false);
        return;
    }

    KLOG_INFO() << "default audio sink:" << m_defaultSinkPath;
    // 线程中不能创建QDBusInterface对象，需要发信号到主线程，由主线程创建
    emit readyToInitDefaultSink();
}

void Volume::initDefaultSink()
{
    m_defaultSink = new QDBusInterface(AUDIO_DBUS_SERVICE, m_defaultSinkPath,
                                       AUDIO_DEVICE_DBUS_INTERFACE_NAME,
                                       QDBusConnection::sessionBus(), this);
    QDBusConnection::sessionBus().connect(
        AUDIO_DBUS_SERVICE, m_defaultSinkPath, PROPERTIES_INTERFACE,
        PROPERTIES_CHANGED, this,
        SLOT(sinkPropertiesChanged(QDBusMessage)));

    initAudioDevice();
}

void Volume::sinkAdded()
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

void Volume::init()
{
    if (m_audioInterface)
    {
        m_audioInterface->deleteLater();
        m_audioInterface = nullptr;
    }

    m_audioInterface = new QDBusInterface(AUDIO_DBUS_SERVICE, AUDIO_OBJECT_PATH,
                                          AUDIO_DBUS_INTERFACE_NAME,
                                          QDBusConnection::sessionBus(), this);
    if (!m_audioInterface->isValid())
    {
        emit enableVolume(false);
        return;
    }

    emit enableVolume(true);

    connect(m_audioInterface, SIGNAL(SinkAdded(uint)), this,
            SLOT(sinkAdded()));
    connect(m_audioInterface, SIGNAL(SinkDelete(uint)), this,
            SLOT(sinkDelete(uint)));
    connect(m_audioInterface, SIGNAL(DefaultSinkChange(uint)), this,
            SLOT(defaultSinkUpdate()));

    defaultSinkUpdate();
}

void Volume::initAudioDevice()
{
    if (!m_defaultSink)
    {
        return;
    }

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
}  // namespace HwConf
}  // namespace Kiran
