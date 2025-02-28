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
enum AppIdType
{
    APP_ID_TYPE_DESKTOP = 0,
    APP_ID_TYPE_WMCLASS
};
class AppInfo
{
public:
    QString m_id;
    AppIdType m_idType;

    QByteArray m_wmClass;
    QUrl m_url;

    AppInfo() = default;

    AppInfo(QUrl url, QByteArray wmClass)
        : m_wmClass(std::move(wmClass)), m_url(std::move(url))
    {
        // 如果desktop文件存在，则使用desktop文件作为id
        // 否则使用wmclass作为id
        if (!m_url.isEmpty() && m_url.isValid())
        {
            m_idType = APP_ID_TYPE_DESKTOP;
            m_id = m_url.toString();
        }
        else
        {
            m_idType = APP_ID_TYPE_WMCLASS;
            m_id = m_wmClass;
        }
    }

    AppInfo &operator=(const AppInfo &other) = default;

    AppInfo(const AppInfo &other)
        : m_id(other.m_id), m_idType(other.m_idType), m_wmClass(other.m_wmClass), m_url(other.m_url) {}

    friend QDebug operator<<(QDebug dbg, const AppInfo &other)
    {
        QDebugStateSaver saver(dbg);

        dbg.nospace() << "AppInfo(" << other.m_id << other.m_wmClass << "," << other.m_url.toString() << ")";
        return dbg.space();
    }

    // 重载==运算
    bool operator==(const AppInfo &other) const
    {
        return m_id == other.m_id;
    }

    bool operator!=(const AppInfo &other) const
    {
        return !(*this == other);
    }

    // 重载<运算，以便map使用
    bool operator<(const AppInfo &other) const
    {
        return m_id < other.m_id;
    }
};

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

    const AppInfo &getAppInfo();
    bool isLocked() const;
    void setLocked(bool lockFlag);

    void setDragData(const QUrl &url);

    bool isOpened() const;

    // 由父控件统一调用
    void updateLayout();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void init();

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
