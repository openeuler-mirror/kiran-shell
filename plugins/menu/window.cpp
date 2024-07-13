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

#include <ks-i.h>
#include <qt5-log-i.h>
#include <unistd.h>
#include <KIO/ApplicationLauncherJob>
#include <KIO/OpenUrlJob>
#include <KWindowSystem>
#include <QApplication>
#include <QButtonGroup>
#include <QDateTime>
#include <QDesktopServices>
#include <QFile>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QProcess>
#include <QSettings>
#include <QStackedWidget>
#include <QStyleOption>
#include <QToolButton>
#include <QKeyEvent>

#include "app-item.h"
#include "apps-overview.h"
#include "ks-config.h"
#include "lib/common/define.h"
#include "lib/common/setting-process.h"
#include "lib/common/utility.h"
#include "power.h"
#include "recent-files-overview.h"
#include "ui_window.h"
#include "window.h"

#define KIRAN_ACCOUNTS_BUS "com.kylinsec.Kiran.SystemDaemon.Accounts"
#define KIRAN_ACCOUNTS_PATH "/com/kylinsec/Kiran/SystemDaemon/Accounts"
#define KIRAN_ACCOUNTS_INTERFACE "com.kylinsec.Kiran.SystemDaemon.Accounts"
#define KIRAN_ACCOUNTS_USER_INTERFACE "com.kylinsec.Kiran.SystemDaemon.Accounts.User"
#define PROPERTIES_INTERFACE "org.freedesktop.DBus.Properties"
#define PROPERTIES_CHANGED "PropertiesChanged"

namespace KAStats = KActivities::Stats;
using namespace KAStats;
using namespace KAStats::Terms;

namespace Kiran
{
namespace Menu
{
Window::Window(QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint),
      m_ui(new Ui::Window),
      m_uid(getuid()),
      m_activitiesConsumer(new KActivities::Consumer()),
      m_accountProxy(nullptr),
      m_accountUserProxy(nullptr),
      m_actStatsLinkedWatcher(nullptr),
      m_actStatsUsedWatcher(nullptr)
{
    m_ui->setupUi(this);

    init();
}

Window::~Window()
{
    delete m_ui;
}

void Window::init()
{
    initUI();

    initActivitiesStats();

    initUserInfo();

    initQuickStart();

    //事件过滤器
    installEventFilter(this);
}

void Window::initUI()
{
    m_ui->m_gridLayoutPopularApp->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_ui->m_gridLayoutFavoriteApp->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    //收藏夹图标
    m_ui->m_btnFavoriteAppIcon->setFlat(true);
    m_ui->m_btnFavoriteAppIcon->setIcon(QIcon::fromTheme(KS_ICON_MENU_GROUP_SYMBOLIC));
    //常用应用图标
    m_ui->m_btnPopularAppIcon->setFlat(true);
    m_ui->m_btnPopularAppIcon->setIcon(QIcon::fromTheme(KS_ICON_MENU_GROUP_SYMBOLIC));

    //应用列表和文件列表切换
    QButtonGroup *overviewSelections = new QButtonGroup(this);
    overviewSelections->addButton(m_ui->m_btnAppsOverview, 0);
    overviewSelections->addButton(m_ui->m_btnRecentFilesOverview, 1);
    // 移除qt designer默认创建的widget
    clear(m_ui->m_widgetOverviewStack);

    AppsOverview *appsOverview = new AppsOverview(this);
    connect(appsOverview, &AppsOverview::isInFavorite, this, &Window::isInFavorite, Qt::DirectConnection);
    connect(appsOverview, &AppsOverview::isInTasklist, this, &Window::isInTasklist, Qt::DirectConnection);
    connect(appsOverview, &AppsOverview::addToFavorite, this, &Window::addToFavorite);
    connect(appsOverview, &AppsOverview::removeFromFavorite, this, &Window::removeFromFavorite);
    connect(appsOverview, &AppsOverview::addToTasklist, this, &Window::addToTasklist);
    connect(appsOverview, &AppsOverview::removeFromTasklist, this, &Window::removeFromTasklist);
    connect(appsOverview, &AppsOverview::addToDesktop, this, &Window::addToDesktop);
    connect(appsOverview, &AppsOverview::runApp, this, &Window::runApp);
    m_ui->m_widgetOverviewStack->addWidget(appsOverview);

    RecentFilesOverview *recentFilesOverview = new RecentFilesOverview(this);
    connect(recentFilesOverview, &RecentFilesOverview::fileItemClicked, this, &Window::openFile);
    m_ui->m_widgetOverviewStack->addWidget(recentFilesOverview);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    connect(overviewSelections, SIGNAL(idClicked(int)), m_ui->m_widgetOverviewStack, SLOT(setCurrentIndex(int)));
#else
    connect(overviewSelections, SIGNAL(buttonClicked(int)), m_ui->m_overviewStack, SLOT(setCurrentIndex(int)));
#endif
}

void Window::initActivitiesStats()
{
    //收藏夹及常用应用服务
    while (m_activitiesConsumer->serviceStatus() == KActivities::Consumer::Unknown)
    {
        QCoreApplication::processEvents();
    }

    //    KLOG_INFO() << activities->activities();
    //    KLOG_INFO() << activities->serviceStatus();
    //    KLOG_INFO() << activities->runningActivities();
    //    KLOG_INFO() << activities->currentActivity();

    //收藏夹监视
    m_actStatsLinkedWatcher = new ResultWatcher(LinkedResources | Agent::global() | Type::any() | Activity::any(), this);
    connect(m_actStatsLinkedWatcher, &ResultWatcher::resultLinked, this, &Window::updateFavorite);
    connect(m_actStatsLinkedWatcher, &ResultWatcher::resultUnlinked, this, &Window::updateFavorite);
    connect(m_actStatsLinkedWatcher, &ResultWatcher::resultScoreUpdated, this, &Window::updateFavorite);
    updateFavorite();

    //常用应用监视
    m_actStatsUsedWatcher = new ResultWatcher(UsedResources | Agent::global() | Type::any() | Activity::any(), this);
    connect(m_actStatsUsedWatcher, &ResultWatcher::resultLinked, this, &Window::updatePopular);
    connect(m_actStatsUsedWatcher, &ResultWatcher::resultUnlinked, this, &Window::updatePopular);
    connect(m_actStatsUsedWatcher, &ResultWatcher::resultScoreUpdated, this, &Window::updatePopular);
    updatePopular();
}

void Window::initUserInfo()
{
    m_ui->m_btnUserPhoto->setFlat(true);

    //用户名、头像
    try
    {
        m_accountProxy = new QDBusInterface(KIRAN_ACCOUNTS_BUS,
                                            KIRAN_ACCOUNTS_PATH,
                                            KIRAN_ACCOUNTS_INTERFACE,
                                            QDBusConnection::systemBus(),
                                            this);
    }
    catch (...)
    {
        KLOG_WARNING() << "new QDBusInterface failed";
    }

    quint64 id = m_uid;
    QDBusMessage msg = m_accountProxy->call("FindUserById", id);
    QString accountUserPath = msg.arguments().at(0).value<QDBusObjectPath>().path();
    bool ret = QDBusConnection::systemBus().connect(KIRAN_ACCOUNTS_BUS,
                                                    accountUserPath,
                                                    PROPERTIES_INTERFACE,
                                                    PROPERTIES_CHANGED,
                                                    this,
                                                    SLOT(userInfoChanged(QDBusMessage)));

    try
    {
        m_accountUserProxy = new QDBusInterface(KIRAN_ACCOUNTS_BUS,
                                                accountUserPath,
                                                KIRAN_ACCOUNTS_USER_INTERFACE,
                                                QDBusConnection::systemBus(),
                                                this);
    }
    catch (...)
    {
        KLOG_WARNING() << "new QDBusInterface failed";
    }

    updateUserInfo();

    //点击头像
    connect(m_ui->m_btnUserPhoto, &QPushButton::clicked, this, [=]()
            { QProcess::startDetached("kiran-control-panel", {"-c", "account-management"}); });
}

void Window::initQuickStart()
{
    //快速启动
    //TODO: mate相关的需要更改成自研

    connect(m_ui->m_btnRunCommand, &QPushButton::clicked, this, [=]()
            { QProcess::startDetached("mate-panel", {"--run-dialog"}); });
    connect(m_ui->m_btnSearchFiles, &QPushButton::clicked, this, [=]()
            { QProcess::startDetached("mate-search-tool", {}); });
    connect(m_ui->m_btnHomeDir, &QPushButton::clicked, this, [=]()
            { QProcess::startDetached("caja", {}); });
    connect(m_ui->m_btnSettings, &QPushButton::clicked, this, [=]()
            { QProcess::startDetached("kiran-control-panel", {}); });
    connect(m_ui->m_btnSystemMonitor, &QPushButton::clicked, this, [=]()
            { QProcess::startDetached("mate-system-monitor", {}); });

    //电源选项
    auto power = Power::getDefault();
    connect(m_ui->m_btnPower, &QPushButton::clicked, this, [=]()
            {
                QMenu power_menu;

                if (power->canLockScreen())
                {
                    power_menu.addAction(QIcon::fromTheme(KS_ICON_MENU_LOCK_SYMBOLIC), tr("Lock screen"), this, [=]()
                                         { power->lockScreen(); });
                }

                if (power->canLogout())
                {
                    power_menu.addAction(QIcon::fromTheme(KS_ICON_POWER_LOGOUT), tr("Logout"), this, [=]()
                                         { power->logout(); });
                }

                if (power->canSwitchUser())
                {
                    power_menu.addAction(QIcon::fromTheme(KS_ICON_POWER_SWITCH_USER), tr("Switch user"), this, [=]()
                                         {
                                             if (power->getGraphicalNtvs() >= power->getNtvsTotal())
                                             {
                                                 KLOG_DEBUG("Total ntvs: %d, graphical ntvs: %d.", power->getNtvsTotal(), power->getGraphicalNtvs());
                                                 //TODO: 弹窗提示，已达最大用户数
                                             }
                                             else
                                             {
                                                 power->switchUser();
                                             }
                                         });
                }

                if (power->canSuspend())
                {
                    power_menu.addAction(QIcon::fromTheme(KS_ICON_POWER_SUSPEND), tr("Suspend"), this, [=]()
                                         { power->suspend(); });
                }

                if (power->canHibernate())
                {
                    power_menu.addAction(QIcon::fromTheme(KS_ICON_POWER_HIBERNATE), tr("Hibernate"), this, [=]()
                                         { power->hibernate(); });
                }

                if (power->canReboot())
                {
                    power_menu.addAction(QIcon::fromTheme(KS_ICON_POWER_REBOOT), tr("Reboot"), this, [=]()
                                         { power->reboot(); });
                }

                if (power->canShutdown())
                {
                    power_menu.addAction(QIcon::fromTheme(KS_ICON_POWER_SHUTDOWN), tr("Shutdown"), this, [=]()
                                         { power->shutdown(); });
                }

                int x = m_ui->m_widgetNavigations->x() + m_ui->m_widgetNavigations->width();
                int y = m_ui->m_widgetNavigations->y() + m_ui->m_widgetNavigations->height();
                power_menu.exec(mapToGlobal(QPoint(x, y)));
            });
}

void Window::clear(QStackedWidget *stackedWidget)
{
    while (stackedWidget->currentWidget() != nullptr)
    {
        auto currentWidget = stackedWidget->currentWidget();
        stackedWidget->removeWidget(currentWidget);
        delete currentWidget;
    }
}

AppItem *Window::newAppItem(QString appId)
{
    AppItem *appItem = new AppItem(this);
    appItem->setAppId(appId);
    connect(appItem, &AppItem::isInFavorite, this, &Window::isInFavorite, Qt::DirectConnection);
    connect(appItem, &AppItem::isInTasklist, this, &Window::isInTasklist, Qt::DirectConnection);
    connect(appItem, &AppItem::addToFavorite, this, &Window::addToFavorite);
    connect(appItem, &AppItem::removeFromFavorite, this, &Window::removeFromFavorite);
    connect(appItem, &AppItem::addToTasklist, this, &Window::addToTasklist);
    connect(appItem, &AppItem::removeFromTasklist, this, &Window::removeFromTasklist);
    connect(appItem, &AppItem::addToDesktop, this, &Window::addToDesktop);
    connect(appItem, &AppItem::runApp, this, &Window::runApp);

    return appItem;
}

void Window::runApp(QString appId)
{
    KService::Ptr service = KService::serviceByStorageId(appId);

    if (service)
    {
        KLOG_INFO() << appId << service->exec();
        //启动应用
        // QProcess::startDetached(service->exec()) service->exec()部分应用带有参数，如%U，导致无法启动
        auto *job = new KIO::ApplicationLauncherJob(service);
        job->start();

        //通知kactivitymanagerd
        KActivities::ResourceInstance::notifyAccessed(QUrl(QStringLiteral("applications:") + service->storageId()));
    }
}

void Window::openFile(QString filePath)
{
    //    auto job = new KIO::OpenUrlJob(file_path);
    //    job->start();

    QDesktopServices::openUrl(filePath);
}

void Window::isInFavorite(const QString &appId, bool &isFavorite)
{
    isFavorite = m_favoriteAppId.contains(appId);
}

void Window::addToFavorite(const QString &appId)
{
    QString appIdTemp = QLatin1String("applications:") + appId;
    KLOG_WARNING() << "addToFavorite" << appIdTemp;
    m_actStatsLinkedWatcher->linkToActivity(QUrl(appIdTemp), Activity::global(), Agent::global());
}

void Window::removeFromFavorite(const QString &appId)
{
    QString appIdTemp = QLatin1String("applications:") + appId;
    m_actStatsLinkedWatcher->unlinkFromActivity(QUrl(appIdTemp), Activity::global(), Agent::global());
}

void Window::isInTasklist(const QUrl &url, bool &checkResult)
{
    checkResult = SettingProcess::isValueInKey(TASKBAR_LOCK_APP_KEY, url);
}

void Window::addToTasklist(const QUrl &url)
{
    SettingProcess::addValueToKey(TASKBAR_LOCK_APP_KEY, url);
}

void Window::removeFromTasklist(const QUrl &url)
{
    SettingProcess::removeValueFromKey(TASKBAR_LOCK_APP_KEY, url);
}

void Window::addToDesktop(const QString &appId)
{
    // TODO:添加到桌面
    QStringList desktopPaths = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation);

    if (desktopPaths.isEmpty())
    {
        KLOG_WARNING() << "Desktop path not found.";
        return;
    }
    QString desktopPath = desktopPaths.first();

    KService::Ptr s = KService::serviceByStorageId(appId);
    if (s)
    {
        QString fileName = QFileInfo(s->entryPath()).fileName();
        QString destPath = QDir::toNativeSeparators(desktopPath + QDir::separator() + fileName);
        QFile::copy(s->entryPath(), destPath);
        if (!QFile(destPath).exists())
        {
            KLOG_WARNING() << "Desktop file copy failed, from" << s->entryPath() << "to" << destPath;
            return;
        }
        QFile::Permissions permissions = QFileInfo(destPath).permissions();
        permissions |= QFile::ExeOwner | QFile::ExeGroup | QFile::ExeOther;
        QFile::setPermissions(destPath, permissions);
    }
}

void Window::updateUserInfo()
{
    //    KLOG_INFO() << "Window::updateUserInfo";

    m_ui->m_labelUserName->setText(tr("Hello,") + qgetenv("USER"));

    QString iconFile = m_accountUserProxy->property("icon_file").toString();
    if (!iconFile.isEmpty() && QFile(iconFile).exists())
    {
        m_ui->m_btnUserPhoto->setIcon(QIcon(iconFile));
    }
    else
    {
        m_ui->m_btnUserPhoto->setIcon(QIcon(":/images/images/avatar_default.png"));
    }
}

void Window::updatePopular()
{
    Utility::clearLayout(m_ui->m_gridLayoutPopularApp, true);

    const auto query = UsedResources | HighScoredFirst | Agent::any() | Type::any() | Activity::any() | Url::startsWith(QStringLiteral("applications:")) | Limit(4);

    //    KLOG_INFO() << "Query: " << query;

    int col = 0;
    for (const ResultSet::Result &result : ResultSet(query))
    {
        //        KLOG_INFO() << result.title();
        QString serviceId = QUrl(result.resource()).path();
        KService::Ptr service = KService::serviceByStorageId(serviceId);
        if (!service || !service->isValid())
        {
            continue;
        }

        AppItem *appItem = newAppItem(serviceId);

        m_ui->m_gridLayoutPopularApp->addWidget(appItem, 0, col++);
    }
}

void Window::updateFavorite()
{
    Utility::clearLayout(m_ui->m_gridLayoutFavoriteApp, true);
    m_favoriteAppId.clear();

    int colMax = 4;
    int rowIndex = 0;
    int colIndex = 0;

    //获取收藏夹数据
    const auto query = LinkedResources | Agent::global() | Type::any() | Activity::any();

    for (const ResultSet::Result &result : ResultSet(query))
    {
        QString serviceId = QUrl(result.resource()).path();
        KService::Ptr service = KService::serviceByStorageId(serviceId);
        if (!service || !service->isValid())
        {
            continue;
        }
        AppItem *appItem = newAppItem(serviceId);

        m_ui->m_gridLayoutFavoriteApp->addWidget(appItem, rowIndex, colIndex++);

        if (colIndex >= colMax)
        {
            colIndex = 0;
            rowIndex++;
        }

        m_favoriteAppId.append(serviceId);
    }
}

void Window::userInfoChanged(QDBusMessage msg)
{
    updateUserInfo();
}

bool Window::eventFilter(QObject *object, QEvent *event)
{
    //window was deactivated
    if (QEvent::WindowDeactivate == event->type())
    {
        emit windowDeactivated();
    }

    return QWidget::eventFilter(object, event);
}

void Window::showEvent(QShowEvent *event)
{
    //任务栏不显示
    KWindowSystem::setState(winId(), NET::SkipTaskbar | NET::SkipPager | NET::SkipSwitcher);
}

void Window::keyPressEvent(QKeyEvent *event)
{
    if (Qt::Key_Up == event->key())
    {
        focusPreviousChild();
    }
    else if (Qt::Key_Down == event->key())
    {
        focusNextChild();
    }
    else if (Qt::Key_Escape == event->key())
    {
        emit windowDeactivated();
    }
    else
    {
        QWidget::keyPressEvent(event);
    }
}

}  // namespace  Menu

}  // namespace Kiran
