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
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#pragma once

#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QPushButton>
#include <QWidget>

#include <KActivities/KActivities/Consumer>
#include <KActivities/KActivities/ResourceInstance>
#include <KActivities/Stats/ResultModel>
#include <KActivities/Stats/ResultSet>
#include <KActivities/Stats/ResultWatcher>
#include <KService/KService>

namespace Ui
{
class Window;
};  // namespace Ui

class QStackedWidget;
class QButtonGroup;
class QStackedWidget;
class QToolButton;
class AppItem;

namespace Kiran
{
namespace Menu
{
class Window : public QWidget
{
    Q_OBJECT

public:
    Window(QWidget* parent = nullptr);
    virtual ~Window();

public slots:
    void userInfoChanged(QDBusMessage msg);

private:
    void init();
    void initUI();
    //收藏夹及常用应用服务
    void initActivitiesStats();
    //用户名和用户头像相关dbus服务初始化
    void initUserInfo();
    //快速启动初始化
    void initQuickStart();

    void clear(QStackedWidget* stackedWidget);

    //启动应用
    void runApp(QString appId);
    //打开文件
    void openFile(QString filePath);

    //收藏夹和常用应用项
    AppItem* newAppItem(QString appId);

    //添加到收藏夹操作
    void isInFavorite(const QString& appId, bool& isFavorite);
    void addToFavorite(const QString& appId);
    void removeFromFavorite(const QString& appId);

    //添加到任务栏操作
    void isInTasklist(const QUrl& url, bool& checkResult);
    void addToTasklist(const QUrl& url);
    void removeFromTasklist(const QUrl& url);

    //添加到桌面操作
    void addToDesktop(const QString& appId);

private slots:
    void updateUserInfo();
    void updateFavorite();
    void updatePopular();

protected:
    //事件过滤器
    bool eventFilter(QObject* object, QEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

signals:
    void windowDeactivated();

private:
    Ui::Window* m_ui;

    //常用应用相关
    std::unique_ptr<KActivities::Consumer> m_activitiesConsumer;
    KActivities::Stats::ResultWatcher* m_actStatsUsedWatcher;

    //收藏夹相关KActivities/Stats/ResultWatcher
    KActivities::Stats::ResultWatcher* m_actStatsLinkedWatcher;
    QStringList m_favoriteAppId;

    //用户id
    uid_t m_uid;
    //用户信息 dbus
    QDBusInterface* m_accountProxy;
    QDBusInterface* m_accountUserProxy;
};
}  // namespace Menu
}  // namespace Kiran
