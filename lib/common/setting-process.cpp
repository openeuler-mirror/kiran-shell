/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
 * kiran-session-manager is licensed under Mulan PSL v2.
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

#include <QSettings>

#include "define.h"
#include "setting-process.h"

QVariant SettingProcess::getValue(QString key)
{
    return getValue(KIRAN_SHELL_SETTING_FILE, key);
}

void SettingProcess::setValue(QString key, const QVariant &value)
{
    setValue(KIRAN_SHELL_SETTING_FILE, key, value);
}

QVariant SettingProcess::getValue(QString iniFile, QString key)
{
    QSettings settings(iniFile, QSettings::IniFormat);
    return settings.value(key).toStringList();
}

void SettingProcess::setValue(QString iniFile, QString key, const QVariant &value)
{
    QSettings settings(iniFile, QSettings::IniFormat);
    settings.setValue(key, value);
}

void SettingProcess::addStringToKey(QString key, const QString &value)
{
    addStringToKey(KIRAN_SHELL_SETTING_FILE, key, value);
}

void SettingProcess::addStringToKey(QString iniFile, QString key, const QString &value)
{
    QSettings settings(iniFile, QSettings::IniFormat);
    QStringList strlistValue = settings.value(key).toStringList();
    if (!strlistValue.contains(value))
    {
        strlistValue.append(value);
        settings.setValue(key, strlistValue);
    }
}

void SettingProcess::removeStringFromKey(QString key, const QString &value)
{
    removeStringFromKey(KIRAN_SHELL_SETTING_FILE, key, value);
}

void SettingProcess::removeStringFromKey(QString iniFile, QString key, const QString &value)
{
    QSettings settings(iniFile, QSettings::IniFormat);
    QStringList strlistValue = settings.value(key).toStringList();
    if (strlistValue.contains(value))
    {
        strlistValue.removeAll(value);
        settings.setValue(key, strlistValue);
    }
}

bool SettingProcess::isStringInKey(QString key, const QString &value)
{
    return isStringInKey(KIRAN_SHELL_SETTING_FILE, key, value);
}

bool SettingProcess::isStringInKey(QString iniFile, QString key, const QString &value)
{
    QSettings settings(iniFile, QSettings::IniFormat);
    QStringList strlistValue = settings.value(key).toStringList();
    return strlistValue.contains(value);
}
