/**
 * Copyright (c) 2020 ~ 2025 KylinSec Co., Ltd.
 * kiran-shell is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     liuxinhao <liuxinhao@kylinsec.com.cn>
 */
#include "tray-settings.h"
#include <QGSettings>
#include <QVariant>
#include "ks-i.h"

namespace Kiran
{
namespace Systemtray
{
TraySettings::TraySettings()
{
    m_settings = new QGSettings(SYSTEMTRAY_SCHEMA_ID);
}

TraySettings::~TraySettings()
{
    delete m_settings;
}

QStringList TraySettings::getFoldingApps() const
{
    return m_settings->get(SYSTEMTRAY_SCHEMA_KEY_FOLDING_APPS).toStringList();
}

bool TraySettings::isFoldingApp(const QString& id) const
{
    return getFoldingApps().contains(id);
}

void TraySettings::addFoldingApp(const QString &id)
{
    QStringList apps = getFoldingApps();
    apps.append(id);
    setFoldingApps(apps);
}

void TraySettings::removeFoldingApp(const QString &id)
{
    QStringList apps = getFoldingApps();
    apps.removeAll(id);
    setFoldingApps(apps);
}

void TraySettings::setFoldingApps(const QStringList &ids)
{
    m_settings->set(SYSTEMTRAY_SCHEMA_KEY_FOLDING_APPS, QVariant::fromValue(ids));
}

}  // namespace Systemtray
}  // namespace Kiran