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

#include <plugin-i.h>
#include <qt5-log-i.h>
#include <QSettings>

#include "app-button-container.h"
#include "lib/common/define.h"
#include "lib/common/setting-process.h"
#include "lib/common/utility.h"
#include "lib/common/window-info-helper.h"

namespace KAStats = KActivities::Stats;
using namespace KAStats;
using namespace KAStats::Terms;

namespace Kiran
{
namespace Taskbar
{
AppButtonContainer::AppButtonContainer(IAppletImport *import, QObject *parent)
    : QObject(parent),
      m_import(import)
{
    loadLockApp();

    QString settingDir = QFileInfo(KIRAN_SHELL_SETTING_FILE).dir().path();
    if (!QDir(settingDir).exists())
    {
        QDir().mkpath(settingDir);
    }
    // QSettings 保存时，会删除原有文件，重新创建一个新文件，所以不能监视文件，此处监视文件夹
    m_settingFileWatcher.addPath(settingDir);
    connect(&m_settingFileWatcher, &QFileSystemWatcher::directoryChanged, this, [=]()
            { updateLockApp(); });

    m_actStatsLinkedWatcher = new ResultWatcher(LinkedResources | Agent::global() | Type::any() | Activity::any(), this);
    connect(m_actStatsLinkedWatcher, &ResultWatcher::resultLinked, this, &AppButtonContainer::updateFavorite);
    connect(m_actStatsLinkedWatcher, &ResultWatcher::resultUnlinked, this, &AppButtonContainer::updateFavorite);
    updateFavorite();
}

AppButtonContainer::~AppButtonContainer()
{
}

void AppButtonContainer::addWindow(WId wid)
{
    QByteArray wmClass = WindowInfoHelper::getWmClassByWId(wid);
    if (wmClass.isEmpty())
    {
        return;
    }

    QByteArray desktopFile = WindowInfoHelper::getDesktopFileByWId(wid);
    if (!desktopFile.isEmpty() && m_mapButtonsLock.contains(desktopFile))
    {
        AppButton *appButton = m_mapButtonsLock.value(desktopFile);
        appButton->setAppInfo(wmClass, wid);
        if (!m_mapButtons.contains(wmClass))
        {
            m_mapButtons.insert(wmClass, appButton);
        }
    }
    else if (!m_mapButtons.contains(wmClass))
    {
        // 创建app按钮
        AppButton *appButton = newAppBtn();
        appButton->setAppInfo(wmClass, wid);
        m_mapButtons.insert(wmClass, appButton);
    }

    m_mapWidWmClass.insert(wid, wmClass);

    emit windowAdded(wmClass, wid);
    emit appRefreshed();
}

void AppButtonContainer::removedWindow(WId wid)
{
    // 此时无法通过wid定位wmclass，使用内存中保存的数据
    if (!m_mapWidWmClass.contains(wid))
    {
        return;
    }

    // 任务栏按钮处理
    QByteArray wmClass = m_mapWidWmClass[wid];
    emit windowRemoved(wmClass, wid);

    // 自身数据处理
    auto iter = m_mapWidWmClass.find(wid);
    m_mapWidWmClass.erase(iter);
}

void AppButtonContainer::changedActiveWindow(WId id)
{
    // TODO:激活按钮，需要集成主题样式
}

QList<AppButton *> AppButtonContainer::getAppButtons()
{
    QList<AppButton *> appButtons = m_mapButtonsLock.values();

    for (AppButton *appBtn : m_mapButtons.values())
    {
        if (!appButtons.contains(appBtn))
        {
            appButtons.push_back(appBtn);
        }
    }

    return appButtons;
}

void AppButtonContainer::removeAppButton()
{
    AppButton *appButton = (AppButton *)sender();

    QByteArray wmClass = m_mapButtons.key(appButton);
    if (wmClass.isEmpty())
    {
        return;
    }

    auto iter = m_mapButtons.find(wmClass);
    m_mapButtons.erase(iter);

    emit appRefreshed();

    if (!m_mapButtonsLock.values().contains(appButton))
    {
        delete appButton;
        appButton = nullptr;
    }
}

void AppButtonContainer::loadLockApp()
{
    QStringList appIds = SettingProcess::getValue(TASKBAR_LOCK_APP_KEY).toStringList();
    for (QString appId : appIds)
    {
        addLockApp(appId);
    }
}

void AppButtonContainer::updateLockApp()
{
    QStringList appIds = SettingProcess::getValue(TASKBAR_LOCK_APP_KEY).toStringList();
    for (QString appId : appIds)
    {
        if (!m_mapButtonsLock.contains(appId))
        {
            addLockApp(appId);
        }
    }

    for (QString appId : m_mapButtonsLock.keys())
    {
        if (!appIds.contains(appId))
        {
            removeLockApp(appId);
        }
    }

    emit appRefreshed();
}

void AppButtonContainer::addLockApp(const QString &appId)
{
    AppButton *appButton = newAppBtn();
    appButton->setAppInfo(appId);
    m_mapButtonsLock.insert(appId, appButton);
}

AppButton *AppButtonContainer::newAppBtn()
{
    AppButton *appButton = new AppButton(m_import, this);
    connect(appButton, &AppButton::windowEmptied, this, &AppButtonContainer::removeAppButton);
    connect(appButton, &AppButton::isInFavorite, this, &AppButtonContainer::isInFavorite, Qt::DirectConnection);
    connect(appButton, &AppButton::isInTasklist, this, &AppButtonContainer::isInTasklist, Qt::DirectConnection);
    connect(appButton, &AppButton::addToFavorite, this, &AppButtonContainer::addToFavorite);
    connect(appButton, &AppButton::removeFromFavorite, this, &AppButtonContainer::removeFromFavorite);
    connect(appButton, &AppButton::addToTasklist, this, &AppButtonContainer::addToTasklist);
    connect(appButton, &AppButton::removeFromTasklist, this, &AppButtonContainer::removeFromTasklist);
    return appButton;
}

void AppButtonContainer::removeLockApp(const QString &appId)
{
    if (m_mapButtonsLock.contains(appId))
    {
        AppButton *appButton = m_mapButtonsLock.take(appId);
        if (!m_mapButtons.values().contains(appButton))
        {
            delete appButton;
            appButton = nullptr;
        }
    }
}

void AppButtonContainer::updateFavorite()
{
    m_favoriteAppId.clear();

    const auto query = LinkedResources | Agent::global() | Type::any() | Activity::any();
    for (const ResultSet::Result &result : ResultSet(query))
    {
        QString serviceId = QUrl(result.resource()).path();
        m_favoriteAppId.append(serviceId);
    }
}

void AppButtonContainer::isInFavorite(const QString &appId, bool &checkResult)
{
    checkResult = m_favoriteAppId.contains(appId);
}

void AppButtonContainer::addToFavorite(const QString &appId)
{
    QString appIdReal = QLatin1String("applications:") + appId;
    KLOG_WARNING() << "addToFavorite" << appIdReal;
    m_actStatsLinkedWatcher->linkToActivity(QUrl(appIdReal), Activity::global(), Agent::global());
}

void AppButtonContainer::removeFromFavorite(const QString &appId)
{
    QString appIdReal = QLatin1String("applications:") + appId;
    m_actStatsLinkedWatcher->unlinkFromActivity(QUrl(appIdReal), Activity::global(), Agent::global());
}

void AppButtonContainer::isInTasklist(const QString &appId, bool &checkResult)
{
    checkResult = SettingProcess::isStringInKey(TASKBAR_LOCK_APP_KEY, appId);
}

void AppButtonContainer::addToTasklist(const QString &appId)
{
    SettingProcess::addStringToKey(TASKBAR_LOCK_APP_KEY, appId);
}

void AppButtonContainer::removeFromTasklist(const QString &appId)
{
    SettingProcess::removeStringFromKey(TASKBAR_LOCK_APP_KEY, appId);
}

}  // namespace Taskbar

}  // namespace Kiran
