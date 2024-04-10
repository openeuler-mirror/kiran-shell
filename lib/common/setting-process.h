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
#pragma once

#include <QObject>

class SettingProcess : public QObject
{
    Q_OBJECT
public:
    static QVariant getValue(const QString &key);
    static void setValue(const QString &key, const QVariant &value);

    static QVariant getValue(const QString &iniFile, const QString &key);
    static void setValue(const QString &iniFile, const QString &key, const QVariant &value);

    static void addValueToKey(const QString &key, const QVariant &value);
    static void addValueToKey(const QString &iniFile, const QString &key, const QVariant &value);
    static void removeValueFromKey(const QString &key, const QVariant &value);
    static void removeValueFromKey(const QString &iniFile, const QString &key, const QVariant &value);
    static bool isValueInKey(const QString &key, const QVariant &value);
    static bool isValueInKey(const QString &iniFile, const QString &key, const QVariant &value);

private:
    SettingProcess() {}
};
