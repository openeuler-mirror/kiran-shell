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
#include "new-apps-manager.h"
#include <QGSettings>
#include <QSet>
#include <QSignalBlocker>
#include <QVariant>
#include "ks-i.h"

namespace Kiran
{
namespace Menu
{
static QSet<QString> list2Set(const QVariantList& list)
{
    QSet<QString> resultSet;
    for (const QVariant& var : list)
    {
        resultSet.insert(var.toString());
    }
    return resultSet;
};

static QVariantList set2List(const QSet<QString>& set)
{
    QVariantList resultList;
    for (const QString& str : set)
    {
        resultList.append(str);
    }
    return resultList;
};

NewAppsManager& NewAppsManager::getInstance()
{
    static NewAppsManager instance;
    return instance;
}

NewAppsManager::NewAppsManager(QObject* parent)
    : QObject(parent)
{
    m_gsettings = new QGSettings(MENU_SCHEMA_ID, "", this);
    connect(m_gsettings, &QGSettings::changed, this, &NewAppsManager::newAppsConfigUpdated);
}

NewAppsManager::~NewAppsManager()
{
}

QSet<QString> NewAppsManager::getAllNewApps() const
{
    auto newApps = m_gsettings->get(MENU_SCHEMA_KEY_NEW_APPS).toList();
    return list2Set(newApps);
}

void NewAppsManager::markAppsAsRead(const QSet<QString>& appIds)
{
    auto newApps = getAllNewApps();
    newApps.subtract(appIds);
    m_gsettings->set(MENU_SCHEMA_KEY_NEW_APPS, set2List(newApps));
}

void NewAppsManager::updateNewApps(const QSet<QString>& newAppIds)
{
    QSignalBlocker blocker(this); // 内部更新新应用集合，不触发信号
    auto currentApps = getAllNewApps();

    // 两个kiran-shell实例同时运行时，并存在两个新应用
    // 两个实例反复更新"新应用"配置(配置内两个应用顺序不一样), 导致异常递归
    if( currentApps.contains(newAppIds) )
    {
        return;
    }

    currentApps.unite(newAppIds);
    m_gsettings->set(MENU_SCHEMA_KEY_NEW_APPS, set2List(currentApps));
}
}  // namespace Menu
}  // namespace Kiran