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

#include <QBoxLayout>
#include <QDebug>
#include <QFileSystemWatcher>
#include <QMap>
#include <QUrl>
#include <QWidget>

#include "lib/common/app-utils.h"

class QGSettings;
namespace Kiran
{
class IAppletImport;

namespace Taskbar
{

class AppButton;
class AppGroup : public QWidget
{
    Q_OBJECT
public:
    explicit AppGroup(IAppletImport *import, const AppInfo &appInfo, QWidget *parent = nullptr);

    // 什么都没有，用于拖拽
    explicit AppGroup(IAppletImport *import, QWidget *parent = nullptr);

    // 增加或关闭窗口
    void addWindow(WId wid);
    void removeWindow(WId wid);
    void changedActiveWindow(WId wid);

    // 获取窗口关联的按钮
    AppButton *getAppButtonByWId(WId wid);
    WId getWidByAppButton(AppButton *appBtn);

    const AppInfo &getAppInfo();
    bool isLocked() const;
    void setLocked(bool lockFlag);

    void setDragData(const QUrl &url);

    bool isOpened() const;

    bool hasWidOnCurrentDesktop();

    // 由父控件统一调用
    void updateLayout();

    void showPreviewer(WId wid);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void init();

    void removeLockApp();

    // 确认是否单个按钮关联了多个窗口
    void getRelationAppSize(int &size);
    void activeRelationApp();
    // 确认在其他桌面是否有关联窗口
    bool isAlsoOpenedOnOtherDesktop();

    void changePreviewerShow(WId wid);
    void windowCloseAll();

    // 新建app按钮
    AppButton *newAppBtn();

    Qt::AlignmentFlag getLayoutAlignment();
    QBoxLayout::Direction getLayoutDirection();

signals:
    // 窗口属性变化
    void windowChanged(WId wid);
    void activeWindowChanged(WId);

    // 预览显示/隐藏
    void previewerShow(QList<WId> wids, QWidget *triggerWidget);
    void previewerHide();
    void previewerShowChange(QList<WId> wids, QWidget *triggerWidget);

    void emptyGroup(AppGroup *);

    // 查询是否在收藏夹中
    void isInFavorite(const QString &appId, bool &checkResult);
    // 查询是否已固定到任务栏
    void isInFixedApps(const QUrl &url, bool &checkResult);
    // 添加到×/从×移除 桌面、收藏夹、任务栏
    void addToFavorite(const QString &appId);
    void removeFromFavorite(const QString &appId);
    void addToFixedApps(const QUrl &url, AppGroup *appGroup);
    void removeFromFixedApps(const QUrl &url);

    // 拖拽移动
    void moveGroupStarted(AppGroup *);
    void moveGroupEnded(AppGroup *);
    void groupMoved(AppGroup *);

private:
    IAppletImport *m_import = nullptr;

    QBoxLayout *m_layout = nullptr;

    AppInfo m_appInfo;
    bool m_isLocked = false;

    QMap<WId, AppButton *> m_mapWidButton;

    AppButton *m_buttonFixed = nullptr;  // 固定按钮，用于锁定显示、拖拽显示

    // 右键拖动起始位置，用于防止误触，当移动坐标达到阈值之后才判定为拖拽
    QPoint dragStartPosition;    // 鼠标按下时的全局坐标
    QPoint buttonStartPosition;  // 按钮在父窗口中的位置

    QGSettings *m_gsettings = nullptr;  // gsettings
};
}  // namespace Taskbar
}  // namespace Kiran
