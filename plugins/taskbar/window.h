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
#include <KWindowSystem>
#include <QBoxLayout>
#include <QFileSystemWatcher>

class QGSettings;
class StyledButton;
namespace Kiran
{
class IAppletImport;

namespace Taskbar
{
class Applet;
class AppGroup;
class AppInfo;
class AppPreviewer;
class Window : public KiranColorBlock
{
    Q_OBJECT
public:
    Window(IAppletImport *import, Applet *parent);
    ~Window() override;

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
    // 初始化相关
    void initUI();
    void initWindowManager();
    void initConfig();

    AppGroup *genAppGroup(const AppInfo &appInfo);

    //  打开或关闭窗口软件
    void addWindow(WId wid);
    void removeWindow(WId wid);
    static bool getAppInfo(WId wid, AppInfo &appInfo);

    // 刷新app显示
    void updateLayout(int showPageIndex = -1);
    void setFixedDimensions(int size, QBoxLayout::Direction direction);
    void adjustAndGetSize(AppGroup *appGroup, QBoxLayout::Direction direction, int &addSize);
    bool shouldCreateNewPage(AppGroup *appGroup, int appSizeCount, int addSize, int totalSize, QBoxLayout::Direction direction);
    void calculateCurrentPageIndex(int showPageIndex);
    void updatePageButtons(QBoxLayout::Direction direction, Qt::AlignmentFlag alignment);

    // 固定到任务栏操作
    void updateLockApp();
    void addLockApp(const QUrl &url);
    void removeLockApp(const AppInfo &info);

    // 收藏夹关联
    void updateFavorite();
    void isInFavorite(const QString &appId, bool &checkResult);
    void addToFavorite(const QString &appId);
    void removeFromFavorite(const QString &appId);
    // 固定到任务栏信息 查询、增加、删除
    void isInTasklist(const QUrl &url, bool &checkResult);
    void addToTasklist(const QUrl &url, AppGroup *appGroup);
    void removeFromTasklist(const QUrl &url);

    // 窗口关闭
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

    void settingChanged(const QString &key);

signals:
    //  打开或关闭窗口软件
    void windowRemoved(WId wid);
    // 窗口属性变化
    void windowChanged(WId, NET::Properties, NET::Properties2);
    // 激活状态
    void activeWindowChanged(WId wid);
    // 预览显示/隐藏
    void previewerShow(QList<WId> wids, QWidget *triggerWidget);
    void previewerHide();
    void previewerShowChange(QList<WId> wids, QWidget *triggerWidget);

private:
    IAppletImport *m_import = nullptr;

    QGSettings *m_gsettings = nullptr;

    QBoxLayout *m_layout = nullptr;

    QMap<AppInfo, QPair<AppGroup *, QList<WId>>> m_mapAppGroupOpened;  // 打开的应用组
    QList<AppGroup *> m_listAppGroupLocked;                            // 锁定应用组
    QList<AppGroup *> m_listAppGroupShow;                              // 所有应用组，用于排序显示

    AppPreviewer *m_appPreviewer = nullptr;  // 应用预览窗口

    QFileSystemWatcher m_settingFileWatcher;  // 用于检测固定到任务栏应用的变化

    // 收藏夹相关KActivities/Stats/ResultWatcher（用于从任务栏右键 添加或移除 收藏夹应用项）
    KActivities::Stats::ResultWatcher *m_actStatsLinkedWatcher = nullptr;
    // 保存收藏夹应用信息，用于判断应用是否在收藏夹中
    QStringList m_favoriteAppId;

    // 拖拽相关
    int m_currentDropIndex = -1;
    AppGroup *m_indicatorWidget = nullptr;

    // 翻页
    StyledButton *m_upPageBtn = nullptr;
    StyledButton *m_downPageBtn = nullptr;
    int m_curPageIndex = -1;             // 当前页序号
    QList<QList<AppGroup *>> m_appPage;  // 　app分页内容,用于拖拽计算位置需要
};

}  // namespace Taskbar

}  // namespace Kiran
