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

#include <QFileSystemWatcher>
#include <QMap>
#include <QPushButton>

#include "app-previewer.h"
#include "style-app-button.h"

namespace Kiran
{
class IAppletImport;

namespace Taskbar
{
class AppButtonContainer;

class AppButton : public StyleAppButton
{
    Q_OBJECT
public:
    AppButton(IAppletImport *import, AppButtonContainer *parent);

    // 新增窗口时调用
    void setAppInfo(QByteArray wmClass, WId wid);
    // 只传desktopfile，用于固定到任务栏的应用（未打开前）
    void setAppInfo(QString appId);

protected:
    void contextMenuEvent(QContextMenuEvent *event);
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);

private:
    // 关联KWindowSystem，增加或关闭窗口
    void addWindow(QByteArray wmClass, WId wid);
    void removeWindow(QByteArray wmClass, WId wid);

    void buttonClicked();

    // 设置名称
    void updateName();

    // 监测窗口变化
    void changedWindow(WId wid, NET::Properties properties, NET::Properties2 properties2);

    void changedActiveWindow(WId wid);
signals:
    // 显示预览窗口
    void previewerShow(QByteArray wmClass, WId wid);
    void previewerHide(QByteArray wmClass, WId wid);

    // 通过按钮进行关闭
    void windowClose(WId wid);

    // 查询是否在收藏夹中
    void isInFavorite(QString appId, bool &checkResult);
    // 查询是否已固定到任务栏
    void isInTasklist(QString appId, bool &checkResult);
    // 添加到×/从×移除 桌面、收藏夹、任务栏
    void addToFavorite(QString appId);
    void removeFromFavorite(QString appId);
    void addToTasklist(QString appId);
    void removeFromTasklist(QString appId);

private:
    IAppletImport *m_import;

    QByteArray m_wmClass;  // AppButton以wmclass为准，每个wmclass关联一个AppButton
    WId m_wid;             // 关联的窗口

    QByteArray m_desktopFile;  // 固定到任务栏时，关联的desktopfile
    QString m_name;            // 应用名称

    QFileSystemWatcher m_settingFileWatcher;  // 用于检测是否显示软件名称
};

}  // namespace Taskbar

}  // namespace Kiran
