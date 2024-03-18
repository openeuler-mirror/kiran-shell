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

#include <KActivities/Stats/ResultSet>
#include <KActivities/Stats/ResultWatcher>
#include <QMap>
#include <QObject>

#include "app-button.h"

namespace Kiran
{
class IAppletImport;

namespace Taskbar
{
class AppButtonContainer : public QObject
{
    Q_OBJECT
public:
    explicit AppButtonContainer(IAppletImport *import, QObject *parent = nullptr);
    ~AppButtonContainer();

    //  打开或关闭窗口软件
    void addWindow(WId wid);
    void removedWindow(WId wid);
    //激活窗口
    void changedActiveWindow(WId id);
    // 获取当前所有app按钮
    QList<AppButton *> getAppButtons();

private:
    // 删除app按钮
    void removeAppButton();
    // 固定到任务栏操作
    void loadLockApp();
    void updateLockApp();
    void addLockApp(const QString &appId);
    void removeLockApp(const QString &appId);
    // 新建app按钮
    AppButton *newAppBtn();
    // 收藏夹关联
    void updateFavorite();
    void isInFavorite(const QString &appId, bool &isFavorite);
    void addToFavorite(const QString &appId);
    void removeFromFavorite(const QString &appId);
    // 固定到任务栏信息 查询、增加、删除
    void isInTasklist(const QString &appId, bool &checkResult);
    void addToTasklist(const QString &appId);
    void removeFromTasklist(const QString &appId);

signals:
    // apps刷新
    void appRefreshed();
    //  打开或关闭窗口软件，用于按钮保存app信息
    void windowAdded(QByteArray wmClass, WId wid);
    void windowRemoved(QByteArray wmClass, WId wid);
    // 窗口属性变化
    void windowChanged(WId, NET::Properties, NET::Properties2);

private:
    IAppletImport *m_import;

    QMap<WId, QByteArray> m_mapWidWmClass;        // 当前打开的软件 key: wid value: wm_class
    QMap<QByteArray, AppButton *> m_mapButtons;   // 当前打开的软件 key: wm_class
    QMap<QString, AppButton *> m_mapButtonsLock;  // 固定到任务栏的软件 key: desktopfile

    QFileSystemWatcher m_settingFileWatcher;  // 用于检测固定到任务栏应用的变化

    // 收藏夹相关KActivities/Stats/ResultWatcher（用于从任务栏右键 添加或移除 收藏夹应用项）
    KActivities::Stats::ResultWatcher *m_actStatsLinkedWatcher;
    // 保存收藏夹应用信息，用于判断应用是否在收藏夹中
    QStringList m_favoriteAppId;
};

}  // namespace Taskbar

}  // namespace Kiran
