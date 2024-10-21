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

#pragma once

#include <kiran-color-block.h>
#include <KActivities/Stats/ResultSet>
#include <KActivities/Stats/ResultWatcher>
#include <QBoxLayout>

#include "app-button.h"

namespace Kiran
{
class IAppletImport;

namespace Taskbar
{
class Applet;
class AppGroup;
class AppBaseInfo;
class AppButtonContainer : public KiranColorBlock
{
    Q_OBJECT
public:
    AppButtonContainer(IAppletImport *import, Applet *parent);
    ~AppButtonContainer();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateLayoutByProfile();

private:
    AppGroup *genAppGroup(const AppBaseInfo &baseinfo);

    //  打开或关闭窗口软件
    void addWindow(WId wid);

    // 刷新app显示
    void updateLayout(int showPageIndex = -1);

    // 固定到任务栏操作
    void updateLockApp();
    void addLockApp(const QUrl &url);
    void removeLockApp(const QUrl &url);

    // 收藏夹关联
    void updateFavorite();
    void isInFavorite(const QString &appId, bool &isFavorite);
    void addToFavorite(const QString &appId);
    void removeFromFavorite(const QString &appId);
    // 固定到任务栏信息 查询、增加、删除
    void isInTasklist(const QUrl &url, bool &checkResult);
    void addToTasklist(const QUrl &url, AppGroup *appGroup);
    void removeFromTasklist(const QUrl &url);

    // 窗口关闭
    void closeWindow(WId wid);
    void removeGroup(AppGroup *group);

    Qt::AlignmentFlag getLayoutAlignment();
    QBoxLayout::Direction getLayoutDirection();

    void openFileByDrop(QDropEvent *event);

    // 拖拽相关，计算位置
    int getInsertedIndex(const QPoint &pos);
    int getMovedIndex(AppGroup *appGroup);

    // 按钮组 拖拽移动
    void startMoveGroup(AppGroup *appGroup);
    void endMoveGroup(AppGroup *appGroup);
    void moveGroup(AppGroup *appGroup);

signals:
    //  打开或关闭窗口软件
    void windowAdded(QByteArray wmClass, WId wid);
    void windowRemoved(WId wid);
    // 窗口属性变化
    void windowChanged(WId, NET::Properties, NET::Properties2);
    // 激活状态
    void activeWindowChanged(WId wid);

private:
    IAppletImport *m_import;
    QBoxLayout *m_layout;

    QMap<QByteArray, AppGroup *> m_mapAppGroupOpened;  // 打开的应用组，key:wm_class
    QList<AppGroup *> m_listAppGroupLocked;            // 锁定应用组
    QList<AppGroup *> m_listAppGroupShow;              // 所有应用组，用于排序显示

    QFileSystemWatcher m_settingFileWatcher;  // 用于检测固定到任务栏应用的变化

    // 收藏夹相关KActivities/Stats/ResultWatcher（用于从任务栏右键 添加或移除 收藏夹应用项）
    KActivities::Stats::ResultWatcher *m_actStatsLinkedWatcher;
    // 保存收藏夹应用信息，用于判断应用是否在收藏夹中
    QStringList m_favoriteAppId;

    // 拖拽相关
    int m_currentDropIndex;
    AppGroup *m_indicatorWidget;

    // 翻页
    StyledButton *m_upPageBtn;
    StyledButton *m_downPageBtn;
    int m_curPageIndex;                  // 当前页序号
    QList<QList<AppGroup *>> m_appPage;  // 　app分页内容,用于拖拽计算位置需要
};

}  // namespace Taskbar

}  // namespace Kiran
