/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
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

#include <QSettings>

#include "define.h"
#include "setting-process.h"

QVariant SettingProcess::getValue(const QString &key)
{
    return getValue(KIRAN_SHELL_SETTING_FILE, key);
}

void SettingProcess::setValue(const QString &key, const QVariant &value)
{
    setValue(KIRAN_SHELL_SETTING_FILE, key, value);
}

QVariant SettingProcess::getValue(const QString &iniFile, const QString &key)
{
    QSettings settings(iniFile, QSettings::IniFormat);
    return settings.value(key);
}

void SettingProcess::setValue(const QString &iniFile, const QString &key, const QVariant &value)
{
    QSettings settings(iniFile, QSettings::IniFormat);
    settings.setValue(key, value);
}

void SettingProcess::addValueToKey(const QString &key, const QVariant &value)
{
    addValueToKey(KIRAN_SHELL_SETTING_FILE, key, value);
}

void SettingProcess::addValueToKey(const QString &iniFile, const QString &key, const QVariant &value)
{
    QSettings settings(iniFile, QSettings::IniFormat);
    QVariantList values = settings.value(key).toList();
    if (!values.contains(value))
    {
        values.append(value);
        settings.setValue(key, values);
    }
}

void SettingProcess::removeValueFromKey(const QString &key, const QVariant &value)
{
    removeValueFromKey(KIRAN_SHELL_SETTING_FILE, key, value);
}

void SettingProcess::removeValueFromKey(const QString &iniFile, const QString &key, const QVariant &value)
{
    QSettings settings(iniFile, QSettings::IniFormat);
    QVariantList values = settings.value(key).toList();
    if (values.contains(value))
    {
        values.removeAll(value);
        settings.setValue(key, values);
    }
}

bool SettingProcess::isValueInKey(const QString &key, const QVariant &value)
{
    return isValueInKey(KIRAN_SHELL_SETTING_FILE, key, value);
}

bool SettingProcess::isValueInKey(const QString &iniFile, const QString &key, const QVariant &value)
{
    QSettings settings(iniFile, QSettings::IniFormat);
    QVariantList values = settings.value(key).toList();
    return values.contains(value);
}
