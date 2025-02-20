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

#include <KWindowInfo>
#include <QBoxLayout>
#include <QDebug>
#include <QFileSystemWatcher>
#include <QMap>
#include <QUrl>
#include <QWidget>

class QGSettings;
namespace Kiran
{
class IAppletImport;

namespace Taskbar
{
class AppBaseInfo
{
public:
    QByteArray m_wmClass;
    QUrl m_url;
    bool m_isLocked = false;

    AppBaseInfo() = default;

    AppBaseInfo(QByteArray wmClass, QUrl url = QUrl(), bool isLocked = false)
        : m_wmClass(std::move(wmClass)), m_url(std::move(url)), m_isLocked(isLocked) {}

    AppBaseInfo &operator=(const AppBaseInfo &other) = default;

    AppBaseInfo(const AppBaseInfo &other) = default;

    friend QDebug operator<<(QDebug dbg, const AppBaseInfo &other)
    {
        QDebugStateSaver saver(dbg);

        dbg.nospace() << "AppBaseInfo(" << other.m_wmClass << "," << other.m_url.toString() << "," << other.m_isLocked << ")";
        return dbg.space();
    }
};

class AppButton;
class AppGroup : public QWidget
{
    Q_OBJECT
public:
    explicit AppGroup(IAppletImport *import, const AppBaseInfo &appBaseInfo, QWidget *parent = nullptr);

    // 什么都没有，用于拖拽
    explicit AppGroup(IAppletImport *import, QWidget *parent = nullptr);

    QUrl getUrl() const;
    bool isLocked() const;
    void setLocked(bool lockFlag);

    void setDragData(const QUrl &url);

    // 由父控件统一调用
    void updateLayout();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void init();

    // 关联KWindowSystem，增加或关闭窗口
    void addWindow(const QByteArray &wmClass, WId wid);
    void removeWindow(WId wid);
    void changedActiveWindow(WId wid);

    void removeLockApp();

    // 确认是否单个按钮关联了多个窗口
    void getRelationAppSize(int &size);

    void showPreviewer(WId wid);
    void changePreviewerShow(WId wid);
    void windowCloseAll();

    // 新建app按钮
    AppButton *newAppBtn();

    Qt::AlignmentFlag getLayoutAlignment();
    QBoxLayout::Direction getLayoutDirection();

signals:
    // 窗口属性变化
    void windowChanged(WId, NET::Properties, NET::Properties2);
    void activeWindowChanged(WId);

    // 预览显示/隐藏
    void previewerShow(QList<WId> wids, QWidget *triggerWidget);
    void previewerHide();
    void previewerShowChange(QList<WId> wids, QWidget *triggerWidget);

    void emptyGroup(AppGroup *);

    // 查询是否在收藏夹中
    void isInFavorite(const QString &appId, bool &checkResult);
    // 查询是否已固定到任务栏
    void isInTasklist(const QUrl &url, bool &checkResult);
    // 添加到×/从×移除 桌面、收藏夹、任务栏
    void addToFavorite(const QString &appId);
    void removeFromFavorite(const QString &appId);
    void addToTasklist(const QUrl &url, AppGroup *appGroup);
    void removeFromTasklist(const QUrl &url);

    // 拖拽移动
    void moveGroupStarted(AppGroup *);
    void moveGroupEnded(AppGroup *);
    void groupMoved(AppGroup *);

private:
    IAppletImport *m_import = nullptr;

    QBoxLayout *m_layout = nullptr;

    AppBaseInfo m_appBaseInfo;

    QMap<WId, AppButton *> m_mapWidButton;

    AppButton *m_buttonFixed = nullptr;  // 固定按钮，用于锁定显示、拖拽显示

    // 右键拖动起始位置，用于防止误触，当移动坐标达到阈值之后才判定为拖拽
    QPoint dragStartPosition;    // 鼠标按下时的全局坐标
    QPoint buttonStartPosition;  // 按钮在父窗口中的位置

    QGSettings *m_gsettings = nullptr;  // gsettings
};
}  // namespace Taskbar
}  // namespace Kiran
