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

#include <KWindowSystem>
#include <QFileSystemWatcher>
#include <QUrl>

#include "app-group.h"
#include "lib/widgets/styled-button.h"

namespace Kiran
{
class IAppletImport;

namespace Taskbar
{
class Window;
class AppButton : public StyledButton
{
    Q_OBJECT
public:
    AppButton(IAppletImport *import, QWidget *parent);

    void setAppInfo(const AppInfo &appInfo);
    void setAppInfo(const AppInfo &appInfo, const WId &wid);  // 新增窗口时调用
    void setUrl(QUrl url);
    void setShowVisualName(const bool &isShow);
    void reset();

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void updateLayout();

private:
    void buttonClicked();

    // 监测窗口变化
    void changedWindow(WId wid, NET::Properties properties, NET::Properties2 properties2);
    void updateShowName();

    void getInfoFromUrl();

    bool checkDropAccept(QPoint pos);
    Qt::AlignmentFlag getLayoutAlignment();

    void setDragFlag(bool flag);

signals:
    // 显示、隐藏预览窗口
    void previewerShowChange(WId wid);
    void previewerShow(WId wid);
    void previewerHide(WId wid);

    // 关闭所有窗口
    void windowCloseAll();

    // 查询是否在收藏夹中
    void isInFavorite(const QString &appId, bool &checkResult);
    // 查询是否已固定到任务栏
    void isInTasklist(const QUrl &url, bool &checkResult);
    // 添加到×/从×移除 x:桌面、收藏夹、任务栏
    void addToFavorite(const QString &appId);
    void removeFromFavorite(const QString &appId);
    void addToTasklist(const QUrl &url, AppButton *appButton);
    void removeFromTasklist(const QUrl &url);

    // 确认是否单个按钮关联了多个窗口
    void getRelationAppSize(int &result);

    void mousePressed(QMouseEvent *event);
    void mouseMoved(QMouseEvent *event);
    void mouseReleased(QMouseEvent *event);

private:
    IAppletImport *m_import;

    AppInfo m_appInfo;

    WId m_wid = 0;  // 关联的窗口

    QFileSystemWatcher m_settingFileWatcher;  // 用于检测是否显示软件名称
    QString m_visualName;                     // 显示的文本
    bool m_isShowName = false;

    bool m_dragFlag = false;  // 当触发组拖动时，不响应按钮的点击事件
};

}  // namespace Taskbar

}  // namespace Kiran
