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

#ifndef APPITEM_H
#define APPITEM_H

#include <kiran-color-block.h>
#include <QAction>
#include <QWidget>

namespace Ui
{
class AppItem;
}

class AppItem : public KiranColorBlock
{
    Q_OBJECT

public:
    explicit AppItem(QWidget *parent = nullptr);
    ~AppItem();

    void setAppId(QString id);

protected:
    void contextMenuEvent(QContextMenuEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

signals:
    // 查询是否在收藏夹中
    void isInFavorite(const QString &appId, bool &checkResult);
    // 查询是否已固定到任务栏
    void isInTasklist(const QString &appId, bool &checkResult);

    // 添加到×/从×移除 桌面、收藏夹、任务栏
    void addToDesktop(const QString &appId);
    void addToFavorite(const QString &appId);
    void removeFromFavorite(const QString &appId);
    void addToTasklist(const QString &appId);
    void removeFromTasklist(const QString &appId);

    // 运行应用
    void runApp(const QString &appId);

private:
    Ui::AppItem *m_ui;

    // 应用id
    QString m_appId;
};

#endif  // APPITEM_H
