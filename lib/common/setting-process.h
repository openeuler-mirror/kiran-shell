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
    static QVariant getValue(QString key);
    static void setValue(QString key, const QVariant &value);

    static QVariant getValue(QString iniFile, QString key);
    static void setValue(QString iniFile, QString key, const QVariant &value);

    static void addStringToKey(QString key, const QString &value);
    static void addStringToKey(QString iniFile, QString key, const QString &value);
    static void removeStringFromKey(QString key, const QString &value);
    static void removeStringFromKey(QString iniFile, QString key, const QString &value);
    static bool isStringInKey(QString key, const QString &value);
    static bool isStringInKey(QString iniFile, QString key, const QString &value);

private:
    SettingProcess() {}
};
