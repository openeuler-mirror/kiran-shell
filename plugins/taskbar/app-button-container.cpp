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

#include <ks-i.h>
#include <plugin-i.h>
#include <qt5-log-i.h>
#include <QSettings>

#include "app-button-container.h"
#include "applet.h"
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
AppButtonContainer::AppButtonContainer(IAppletImport *import, Applet *parent)
    : QWidget(parent),
      m_import(import)
{
    connect(parent, &Applet::windowAdded, this, &AppButtonContainer::addWindow);
    connect(parent, &Applet::windowRemoved, this, &AppButtonContainer::removeWindow);
    connect(parent, &Applet::activeWindowChanged, this, &AppButtonContainer::changeActiveWindow);

    auto direction = getLayoutDirection();
    m_layout = new QBoxLayout(direction, this);
    m_layout->setSpacing(0);
    m_layout->setMargin(0);

    QObject *Object = dynamic_cast<QObject *>(m_import->getPanel());
    bool ret = connect(Object, SIGNAL(panelProfileChanged()), this, SLOT(updateAppShow()));

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

    m_appPreviewer = new AppPreviewer(m_import, this);
    connect(m_appPreviewer, &AppPreviewer::windowClose, this, &AppButtonContainer::closeWindow);
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

    auto widAndAppButton = m_mapButtons.values(wmClass);
    for (auto iter : widAndAppButton)
    {
        if (iter.first == wid)
        {
            // 已经存在窗口
            return;
        }
    }

    AppButton *appButton = nullptr;
    QByteArray desktopFile = WindowInfoHelper::getDesktopFileByWId(wid);
    if (!desktopFile.isEmpty() && m_mapButtonsLock.contains(desktopFile))
    {
        appButton = m_mapButtonsLock.value(desktopFile);
    }
    else
    {
        // 创建app按钮
        appButton = newAppBtn();
    }

    appButton->setAppInfo(wmClass, wid);
    m_mapButtons.insert(wmClass, {wid, appButton});

    emit windowAdded(wmClass, wid);
    updateAppShow();
}

void AppButtonContainer::removeWindow(WId wid)
{
    AppButton *appButton = nullptr;
    QByteArray wmClass;

    auto iter = m_mapButtons.begin();
    while (iter != m_mapButtons.end())
    {
        if (iter.value().first == wid)
        {
            wmClass = iter.key();
            appButton = iter.value().second;
            break;
        }
        iter++;
    }

    if (wmClass.isEmpty() || !appButton)
    {
        return;
    }

    m_mapButtons.remove(wmClass, {wid, appButton});

    if (!m_mapButtonsLock.values().contains(appButton))
    {
        delete appButton;
        appButton = nullptr;
    }

    emit windowRemoved(wmClass, wid);

    updateAppShow();
}

void AppButtonContainer::changeActiveWindow(WId wid)
{
    emit activeWindowChanged(wid);
    // TODO:激活按钮，需要集成主题样式

    AppButton *appButton = nullptr;
    QByteArray wmClass;

    auto iter = m_mapButtons.begin();
    while (iter != m_mapButtons.end())
    {
        if (iter.value().first == wid)
        {
            wmClass = iter.key();
            appButton = iter.value().second;
            //            break;
        }
        iter.value().second->setChecked(false);
        iter++;
    }

    if (wmClass.isEmpty() || !appButton)
    {
        return;
    }

    if (SettingProcess::getValue(TASKBAR_SHOW_APP_BTN_TAIL_KEY).toBool())
    {
        appButton->setChecked(true);
    }
    else
    {
        QList<QPair<WId, AppButton *>> values = m_mapButtons.values(wmClass);
        if (!values.isEmpty())
        {
            values.first().second->setChecked(true);
        }
    }
}

void AppButtonContainer::updateAppShow()
{
    Utility::clearLayout(m_layout, false, true);

    //横竖摆放
    auto direction = getLayoutDirection();
    m_layout->setDirection(direction);
    //子控件对齐方式：左右、上下
    Qt::AlignmentFlag alignment = getLayoutAlignment();
    m_layout->setAlignment(alignment);

    QList<AppButton *> appButtons = m_mapButtonsLock.values();

    // 根据当前模式，显示不一样的结果
    if (SettingProcess::getValue(TASKBAR_SHOW_APP_BTN_TAIL_KEY).toBool())
    {
        for (auto iter : m_mapButtons.values())
        {
            if (!appButtons.contains(iter.second))
            {
                appButtons.push_back(iter.second);
            }
        }
    }
    else
    {
        for (auto key : m_mapButtons.keys())
        {
            QList<QPair<WId, AppButton *>> values = m_mapButtons.values(key);
            if (!values.isEmpty())
            {
                appButtons.push_back(values.first().second);
            }
        }
    }

    for (auto button : appButtons)
    {
        button->show();
        m_layout->addWidget(button);
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

    updateAppShow();
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
    connect(appButton, &AppButton::previewerShow, this, &AppButtonContainer::showPreviewer);
    connect(appButton, &AppButton::previewerHide, this, &AppButtonContainer::previewerHide);
    connect(appButton, &AppButton::windowClose, this, &AppButtonContainer::closeWindow);
    connect(appButton, &AppButton::isInFavorite, this, &AppButtonContainer::isInFavorite, Qt::DirectConnection);
    connect(appButton, &AppButton::isInTasklist, this, &AppButtonContainer::isInTasklist, Qt::DirectConnection);
    connect(appButton, &AppButton::addToFavorite, this, &AppButtonContainer::addToFavorite);
    connect(appButton, &AppButton::removeFromFavorite, this, &AppButtonContainer::removeFromFavorite);
    connect(appButton, &AppButton::addToTasklist, this, &AppButtonContainer::addToTasklist);
    connect(appButton, &AppButton::removeFromTasklist, this, &AppButtonContainer::removeFromTasklist);
    // 需要显示时才显示
    appButton->hide();

    return appButton;
}

void AppButtonContainer::removeLockApp(const QString &appId)
{
    if (m_mapButtonsLock.contains(appId))
    {
        AppButton *appButton = m_mapButtonsLock.take(appId);
        bool isfind = false;
        for (auto value : m_mapButtons.values())
        {
            if (value.second == appButton)
            {
                isfind = true;
                break;
            }
        }
        if (!isfind)
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

void AppButtonContainer::closeWindow(WId wid)
{
    WindowInfoHelper::closeWindow(wid);
    m_appPreviewer->hide();
}

QBoxLayout::Direction AppButtonContainer::getLayoutDirection()
{
    int orientation = m_import->getPanel()->getOrientation();
    auto direction = (orientation == PanelOrientation::PANEL_ORIENTATION_BOTTOM ||
                      orientation == PanelOrientation::PANEL_ORIENTATION_TOP)
                         ? QBoxLayout::Direction::LeftToRight
                         : QBoxLayout::Direction::TopToBottom;
    return direction;
}

void AppButtonContainer::showPreviewer(QByteArray wmClass, WId wid)
{
    AppButton *button = (AppButton *)sender();
    QPoint center = mapToGlobal(button->geometry().center());
    emit previewerShow(wmClass, wid, center);
}

Qt::AlignmentFlag AppButtonContainer::getLayoutAlignment()
{
    int orientation = m_import->getPanel()->getOrientation();
    Qt::AlignmentFlag alignment = (orientation == PanelOrientation::PANEL_ORIENTATION_BOTTOM ||
                                   orientation == PanelOrientation::PANEL_ORIENTATION_TOP)
                                      ? Qt::AlignLeft
                                      : Qt::AlignTop;

    return alignment;
}

}  // namespace Taskbar

}  // namespace Kiran
