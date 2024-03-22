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
#include <QWidget>

#include "app-button.h"

namespace Kiran
{
class IAppletImport;

namespace Taskbar
{
class Applet;
class AppButtonContainer : public QWidget
{
    Q_OBJECT
public:
    AppButtonContainer(IAppletImport *import, Applet *parent);
    ~AppButtonContainer();

private slots:
    // 刷新app显示
    void updateAppShow();

private:
    //  打开或关闭窗口软件
    void addWindow(WId wid);
    void removeWindow(WId wid);
    //激活窗口
    void changeActiveWindow(WId wid);

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

    // 窗口关闭
    void closeWindow(WId wid);

    Qt::AlignmentFlag getLayoutAlignment();
    QBoxLayout::Direction getLayoutDirection();

    void showPreviewer(QByteArray wmClass, WId wid);

signals:
    //  打开或关闭窗口软件
    void windowAdded(QByteArray wmClass, WId wid);
    void windowRemoved(QByteArray wmClass, WId wid);
    // 窗口属性变化
    void windowChanged(WId, NET::Properties, NET::Properties2);
    // 激活状态
    void activeWindowChanged(WId wid);

    // 预览显示/隐藏
    void previewerShow(QByteArray wmClass, WId wid, QPoint centerOnGlobal);
    void previewerHide(QByteArray wmClass, WId wid);

private:
    IAppletImport *m_import;

    QMultiMap<QByteArray, QPair<WId, AppButton *>> m_mapButtons;  // key:wm_class
    QMap<QString, AppButton *> m_mapButtonsLock;                  // 固定到任务栏的软件 key: desktopfile

    AppPreviewer *m_appPreviewer;  // 应用预览窗口

    QFileSystemWatcher m_settingFileWatcher;  // 用于检测固定到任务栏应用的变化

    // 收藏夹相关KActivities/Stats/ResultWatcher（用于从任务栏右键 添加或移除 收藏夹应用项）
    KActivities::Stats::ResultWatcher *m_actStatsLinkedWatcher;
    // 保存收藏夹应用信息，用于判断应用是否在收藏夹中
    QStringList m_favoriteAppId;

    QBoxLayout *m_layout;
};

}  // namespace Taskbar

}  // namespace Kiran
