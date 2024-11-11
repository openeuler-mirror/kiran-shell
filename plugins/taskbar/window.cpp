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

#include <kiran-color-block.h>

#include <ks-i.h>
#include <plugin-i.h>
#include <qt5-log-i.h>
#include <KActivities/KActivities/ResourceInstance>
#include <KActivities/Stats/ResultSet>
#include <KActivities/Stats/ResultWatcher>
#include <KIO/ApplicationLauncherJob>
#include <KIOCore/KFileItem>
#include <KService/KService>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QPainter>
#include <QSettings>
#include <QTimer>

#include "app-button.h"
#include "app-group.h"
#include "applet.h"
#include "lib/common/define.h"
#include "lib/common/setting-process.h"
#include "lib/common/utility.h"
#include "lib/common/window-info-helper.h"
#include "lib/common/window-manager.h"
#include "window.h"

namespace KAStats = KActivities::Stats;
using namespace KAStats;
using namespace KAStats::Terms;

static const int appMargin = 4;
static const int appSpacing = 8;

namespace Kiran
{
namespace Taskbar
{
Window::Window(IAppletImport *import, Applet *parent)
    : KiranColorBlock(parent),
      m_import(import),
      m_indicatorWidget(nullptr),
      m_curPageIndex(-1)
{
    auto direction = getLayoutDirection();
    m_layout = new QBoxLayout(direction, this);
    m_layout->setMargin(appMargin);
    m_layout->setSpacing(appSpacing);
    setLayout(m_layout);

    setRadius(0);

    // 翻页按钮
    m_upPageBtn = new StyledButton(this);
    m_upPageBtn->setIcon(QIcon::fromTheme(KS_ICON_TASKLIST_UP_PAGE_SYMBOLIC));
    m_upPageBtn->hide();
    m_downPageBtn = new StyledButton(this);
    m_downPageBtn->setIcon(QIcon::fromTheme(KS_ICON_TASKLIST_DOWN_PAGE_SYMBOLIC));
    m_downPageBtn->hide();
    connect(m_upPageBtn, &QAbstractButton::clicked, [this]()
            {
                int showPageIndex = m_curPageIndex - 1;
                if (showPageIndex < 0)
                {
                    showPageIndex = 0;
                }
                updateLayout(showPageIndex);
            });
    connect(m_downPageBtn, &QAbstractButton::clicked, [this]()
            {
                updateLayout(m_curPageIndex + 1);
            });

    QObject *Object = dynamic_cast<QObject *>(m_import->getPanel());
    connect(Object, SIGNAL(panelProfileChanged()), this, SLOT(updateLayoutByProfile()));

    updateLockApp();

    QString settingDir = QFileInfo(KIRAN_SHELL_SETTING_FILE).dir().path();
    // QSettings 保存时，会删除原有文件，重新创建一个新文件，所以不能监视文件，此处监视文件夹
    m_settingFileWatcher.addPath(settingDir);
    connect(&m_settingFileWatcher, &QFileSystemWatcher::directoryChanged, this, [=]()
            {
                for (auto appGroup : m_listAppGroupShow)
                {
                    appGroup->updateLayout();
                }
                updateLockApp();
                updateLayout();
            });

    m_actStatsLinkedWatcher = new ResultWatcher(LinkedResources | Agent::global() | Type::any() | Activity::any(), this);
    connect(m_actStatsLinkedWatcher, &ResultWatcher::resultLinked, this, &Window::updateFavorite);
    connect(m_actStatsLinkedWatcher, &ResultWatcher::resultUnlinked, this, &Window::updateFavorite);
    updateFavorite();

    connect(parent, &Applet::windowAdded, this, &Window::addWindow);
    connect(parent, &Applet::windowRemoved, this, &Window::windowRemoved);
    connect(parent, &Applet::activeWindowChanged, [this](WId wid)
            {
                updateLayout();
                emit activeWindowChanged(wid);
            });

    // 等待应用加载
    // NOTE: 需要多测,看会不会延迟加载
    //    QTimer::singleShot(1000, this, [this]()
    //                       {
    //                           for (auto wid : WindowManagerInstance.getAllWindow())
    //                           {
    //                               addWindow(wid);
    //                           }
    //                           WId wid = WindowInfoHelper::activeWindow();
    //                           emit activeWindowChanged(wid);
    //                       });

    for (auto wid : WindowManagerInstance.getAllWindow())
    {
        addWindow(wid);
    }
    WId wid = WindowInfoHelper::activeWindow();
    emit activeWindowChanged(wid);

    setAcceptDrops(true);
    m_currentDropIndex = -1;
    m_indicatorWidget = new AppGroup(m_import, this);
    m_indicatorWidget->hide();

    updateLayout();
}

Window::~Window()
{
}

void Window::dragEnterEvent(QDragEnterEvent *event)
{
    m_currentDropIndex = -1;
    QByteArray mimeData = event->mimeData()->data("text/uri-list");
    if (event->mimeData()->hasFormat("text/uri-list"))
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void Window::dragMoveEvent(QDragMoveEvent *event)
{
    QPoint pos = event->pos();
    m_currentDropIndex = getInsertedIndex(pos);
    if (-1 == m_currentDropIndex)
    {
        m_listAppGroupShow.removeAll(m_indicatorWidget);
        m_indicatorWidget->hide();
        updateLayout();

        event->accept();
        return;
    }

    if (m_currentDropIndex != m_listAppGroupShow.indexOf(m_indicatorWidget))
    {
        m_listAppGroupShow.removeAll(m_indicatorWidget);

        if (m_currentDropIndex >= m_listAppGroupShow.size())
        {
            m_listAppGroupShow.append(m_indicatorWidget);
        }
        else
        {
            m_listAppGroupShow.insert(m_currentDropIndex, m_indicatorWidget);
        }

        m_indicatorWidget->show();
    }

    updateLayout();

    event->accept();
}

void Window::dragLeaveEvent(QDragLeaveEvent *event)
{
    //    KLOG_INFO() << "AppButtonContainer::dragLeaveEvent";
    m_currentDropIndex = -1;
    m_listAppGroupShow.removeAll(m_indicatorWidget);
    m_indicatorWidget->hide();
    updateLayout();

    event->accept();
}

void Window::dropEvent(QDropEvent *event)
{
    KLOG_INFO() << "AppButtonContainer::dropEvent";

    if (-1 == m_currentDropIndex)
    {
        // 打开文件
        openFileByDrop(event);

        event->accept();
        return;
    }

    QByteArray mimeData = event->mimeData()->data("text/uri-list");
    if (!event->mimeData()->hasFormat("text/uri-list"))
    {
        event->accept();
        return;
    }

    m_listAppGroupShow.removeAll(m_indicatorWidget);
    m_indicatorWidget->hide();

    AppBaseInfo appBaseinfo;

    QList<QUrl> urls(event->mimeData()->urls());
    KLOG_INFO() << "AppButtonContainer::dropEvent" << urls;
    for (auto url : urls)
    {
        KFileItem fileItem(url);
        if (fileItem.isNull())
        {
            continue;
        }

        appBaseinfo.m_url = url;
        break;
    }

    if (appBaseinfo.m_url.isEmpty())
    {
        event->accept();
        return;
    }

    AppGroup *appGroup = genAppGroup(appBaseinfo);

    // 如果 appGroup 本来就在任务栏显示，调整位置，不锁定
    if (m_listAppGroupShow.contains(appGroup))
    {
        int dropIndex = m_currentDropIndex;
        if (dropIndex >= m_listAppGroupShow.size())
        {
            dropIndex = m_listAppGroupShow.size() - 1;
        }
        m_listAppGroupShow.move(m_listAppGroupShow.indexOf(appGroup), dropIndex);
        // 上面将显示排序已确定
        // 已锁定应用调整位置
        if (m_listAppGroupLocked.contains(appGroup))
        {
            QList<AppGroup *> listAppGroupLocked;
            for (auto appGroup : m_listAppGroupShow)
            {
                if (m_listAppGroupLocked.contains(appGroup))
                {
                    listAppGroupLocked.append(appGroup);
                }
            }
            m_listAppGroupLocked = listAppGroupLocked;
            addToTasklist(appBaseinfo.m_url, appGroup);
        }
    }
    else
    {
        // 直接使用insert，效果一样，但是Qt有警告
        if (m_currentDropIndex >= m_listAppGroupShow.size())
        {
            m_listAppGroupShow.append(appGroup);
        }
        else
        {
            m_listAppGroupShow.insert(m_currentDropIndex, appGroup);
        }

        addToTasklist(appBaseinfo.m_url, appGroup);
    }

    updateLayout();

    event->accept();
}

void Window::showEvent(QShowEvent *event)
{
    updateLayout();
}

void Window::resizeEvent(QResizeEvent *event)
{
    updateLayout();
}

void Window::updateLayoutByProfile()
{
    for (auto appGroup : m_listAppGroupShow)
    {
        appGroup->updateLayout();
    }
    updateLayout();
}

AppGroup *Window::genAppGroup(const AppBaseInfo &baseinfo)
{
    AppGroup *appGroup = nullptr;

    // 是否是已固定应用
    for (auto iter : m_listAppGroupLocked)
    {
        if (iter->getUrl() == baseinfo.m_url)
        {
            appGroup = iter;
            break;
        }
    }

    if (appGroup)
    {
        return appGroup;
    }

    // 是否是已打开应用
    for (auto iter : m_mapAppGroupOpened)
    {
        if (iter->getUrl() == baseinfo.m_url)
        {
            appGroup = iter;
            break;
        }
    }

    if (appGroup)
    {
        return appGroup;
    }

    if (!appGroup)
    {
        appGroup = new AppGroup(m_import, baseinfo, this);
    }

    connect(appGroup, &AppGroup::isInFavorite, this, &Window::isInFavorite, Qt::DirectConnection);
    connect(appGroup, &AppGroup::isInTasklist, this, &Window::isInTasklist, Qt::DirectConnection);
    connect(appGroup, &AppGroup::addToFavorite, this, &Window::addToFavorite);
    connect(appGroup, &AppGroup::removeFromFavorite, this, &Window::removeFromFavorite);
    connect(appGroup, &AppGroup::addToTasklist, this, &Window::addToTasklist);
    connect(appGroup, &AppGroup::removeFromTasklist, this, &Window::removeFromTasklist);
    connect(appGroup, &AppGroup::emptyGroup, this, &Window::removeGroup);

    connect(appGroup, &AppGroup::moveGroupStarted, this, &Window::startMoveGroup, Qt::QueuedConnection);
    connect(appGroup, &AppGroup::moveGroupEnded, this, &Window::endMoveGroup, Qt::QueuedConnection);
    connect(appGroup, &AppGroup::groupMoved, this, &Window::moveGroup, Qt::QueuedConnection);

    return appGroup;
}

void Window::addWindow(WId wid)
{
    QByteArray wmClass = WindowInfoHelper::getWmClassByWId(wid);
    if (wmClass.isEmpty())
    {
        KLOG_WARNING() << "can't find wmclass by wid:" << wid;
        return;
    }

    QUrl url = WindowInfoHelper::getUrlByWId(wid);

    //    KLOG_INFO() << "AppButtonContainer::addWindow:" << wid << wmClass << url;

    // 锁定应用的打开
    AppGroup *appGroup = nullptr;
    if (!url.isEmpty())
    {
        for (auto appLocked : m_listAppGroupLocked)
        {
            if (appLocked->getUrl() == url)
            {
                appGroup = appLocked;
                m_mapAppGroupOpened[wmClass] = appGroup;
                break;
            }
        }
    }

    if (!appGroup)
    {
        if (!m_mapAppGroupOpened.contains(wmClass))
        {
            // 创建组
            AppBaseInfo appBaseInfo(wmClass, url);
            appGroup = genAppGroup(appBaseInfo);
            m_mapAppGroupOpened[wmClass] = appGroup;
            m_listAppGroupShow.append(appGroup);
        }
        else
        {
            appGroup = m_mapAppGroupOpened[wmClass];
        }
    }

    emit windowAdded(wmClass, wid);
    updateLayout();
}

void Window::updateLayout(int showPageIndex)
{
    Utility::clearLayout(m_layout, false, false);

    //横竖摆放
    auto direction = getLayoutDirection();
    m_layout->setDirection(direction);
    //子控件对齐方式：左右、上下
    Qt::AlignmentFlag alignment = getLayoutAlignment();
    m_layout->setAlignment(alignment);

    auto size = m_import->getPanel()->getSize();

    if (QBoxLayout::Direction::LeftToRight == direction)
    {
        // 清理之前设置的fixed大小
        setMaximumWidth(QWIDGETSIZE_MAX);
        // 重新设置
        setFixedHeight(size);
    }
    else
    {
        setMaximumHeight(QWIDGETSIZE_MAX);
        setFixedWidth(size);
    }

    int appSizeCount = appMargin * 2;  //　累计app宽或高

    m_appPage.clear();
    m_appPage.push_back(QList<AppGroup *>());  //　新建空页

    int totalSize = width();
    if (QBoxLayout::Direction::LeftToRight != direction)
    {
        totalSize = height();
    }

    int pageBtnSize = m_upPageBtn->width() * 2;
    for (auto appGroup : m_listAppGroupShow)
    {
        // 为什么先调整大小?
        // 1.AppButton大小变化 2.当前窗口缩放时,引起AppGroup缩放
        // 以上缩放不会马上应用到实际大小,导致获取大小有偏差
        appGroup->adjustSize();
        //        KLOG_INFO() <<"appGroup show size:" appGroup->size();

        int addSize = 0;
        if (QBoxLayout::Direction::LeftToRight == direction)
        {
            addSize += appGroup->width();
        }
        else
        {
            addSize += appGroup->height();
        }
        addSize += appSpacing;

        // 如果是第一页最后一个app,不需要提前将翻页按钮大小计算进去,可能不需要显示翻页按钮
        if (1 == m_appPage.size() && appGroup == m_listAppGroupShow.last())
        {
            if (appSizeCount + addSize > totalSize)
            {
                m_appPage.push_back(QList<AppGroup *>());  // 超出页面，新建空页
                appSizeCount = appMargin * 2;
            }
        }
        else
        {
            if (appSizeCount + addSize + m_upPageBtn->width() * 2 > totalSize)
            {
                m_appPage.push_back(QList<AppGroup *>());  // 超出页面，新建空页
                appSizeCount = appMargin * 2;
            }
        }

        appSizeCount += addSize;
        m_appPage.last().push_back(appGroup);
    }

    // 计算当前页面序号
    // 获取当前激活窗口信息
    // 判断位于哪页，就是当前页面序号
    if (-1 == showPageIndex)
    {
        // 未指定显示哪页,按照窗口激活情况进行显示
        if (1 == m_appPage.size())
        {
            m_curPageIndex = 0;
        }
        else
        {
            QByteArray wmClass;
            auto wid = WindowInfoHelper::activeWindow();
            if (0 != wid)
            {
                wmClass = WindowInfoHelper::getWmClassByWId(wid);
            }
            if (wmClass.isEmpty() || !m_mapAppGroupOpened.contains(wmClass))
            {
                m_curPageIndex = 0;
            }
            else
            {
                auto appGroup = m_mapAppGroupOpened[wmClass];
                for (m_curPageIndex = 0; m_curPageIndex < m_appPage.size(); m_curPageIndex++)
                {
                    if (m_appPage[m_curPageIndex].contains(appGroup))
                    {
                        break;
                    }
                }
            }
        }
    }
    else
    {
        // 指定了显示哪页
        if (showPageIndex >= m_appPage.size())
        {
            m_curPageIndex = m_appPage.size() - 1;
        }
        else
        {
            m_curPageIndex = showPageIndex;
        }
    }

    // 显示对应页
    for (auto appGroup : m_appPage[m_curPageIndex])
    {
        appGroup->show();
        m_layout->addWidget(appGroup);
    }

    // 其他应用隐藏
    for (auto appGroup : m_listAppGroupShow)
    {
        if (!m_appPage[m_curPageIndex].contains(appGroup))
        {
            appGroup->hide();
        }
    }

    // 翻页按钮
    if (m_appPage.size() > 1)
    {
        QBoxLayout *layoutPage = new QBoxLayout(direction);
        layoutPage->setMargin(0);
        layoutPage->setSpacing(0);
        layoutPage->addWidget(m_upPageBtn);
        layoutPage->addWidget(m_downPageBtn);
        layoutPage->setDirection(direction);
        layoutPage->setAlignment(alignment);

        m_layout->addStretch(1);
        m_layout->addItem(layoutPage);
        m_upPageBtn->show();
        m_downPageBtn->show();
    }
    else
    {
        m_upPageBtn->hide();
        m_downPageBtn->hide();
    }

    // 强制重布局
    // 解决鼠标拖动过程中，鼠标偏离控件，会导致布局不能及时更新
    m_layout->activate();
}

void Window::updateLockApp()
{
    QVariantList appUrls = SettingProcess::getValue(TASKBAR_LOCK_APP_KEY).toList();

    for (auto appUrl : appUrls)
    {
        addLockApp(appUrl.toUrl());
    }

    for (auto appGroup : m_listAppGroupLocked)
    {
        if (!appUrls.contains(appGroup->getUrl()))
        {
            removeLockApp(appGroup->getUrl());
        }
    }
}

void Window::addLockApp(const QUrl &url)
{
    AppBaseInfo baseinfo;
    baseinfo.m_url = url;
    baseinfo.m_isLocked = true;
    AppGroup *appGroup = genAppGroup(baseinfo);
    if (!appGroup)
    {
        return;
    }

    if (!m_listAppGroupLocked.contains(appGroup))
    {
        m_listAppGroupLocked.append(appGroup);
    }

    if (!m_listAppGroupShow.contains(appGroup))
    {
        // 不需要调整位置，只有初次加载时，才会走这里
        m_listAppGroupShow.append(appGroup);
    }

    updateLayout();
}

void Window::removeLockApp(const QUrl &url)
{
    AppGroup *appGroup = nullptr;
    for (auto app : m_listAppGroupLocked)
    {
        if (app->getUrl() == url)
        {
            appGroup = app;
            break;
        }
    }

    if (appGroup)
    {
        m_listAppGroupLocked.removeAll(appGroup);
        appGroup->setLocked(false);

        auto iter = m_mapAppGroupOpened.begin();
        while (iter != m_mapAppGroupOpened.end())
        {
            if (iter.value() == appGroup)
            {
                break;
            }
            iter++;
        }

        if (iter == m_mapAppGroupOpened.end())
        {
            removeGroup(appGroup);
        }
    }
    else
    {
        KLOG_ERROR() << "can't find lock app:appId";
    }
}

void Window::updateFavorite()
{
    m_favoriteAppId.clear();

    const auto query = LinkedResources | Agent::global() | Type::any() | Activity::any();
    for (const ResultSet::Result &result : ResultSet(query))
    {
        QString serviceId = QUrl(result.resource()).path();
        m_favoriteAppId.append(serviceId);
    }
}

void Window::isInFavorite(const QString &appId, bool &checkResult)
{
    checkResult = m_favoriteAppId.contains(appId);
}

void Window::addToFavorite(const QString &appId)
{
    QString appIdReal = QLatin1String("applications:") + appId;
    KLOG_WARNING() << "addToFavorite" << appIdReal;
    m_actStatsLinkedWatcher->linkToActivity(QUrl(appIdReal), Activity::global(), Agent::global());
}

void Window::removeFromFavorite(const QString &appId)
{
    QString appIdReal = QLatin1String("applications:") + appId;
    m_actStatsLinkedWatcher->unlinkFromActivity(QUrl(appIdReal), Activity::global(), Agent::global());
}

void Window::isInTasklist(const QUrl &url, bool &checkResult)
{
    checkResult = SettingProcess::isValueInKey(TASKBAR_LOCK_APP_KEY, url);
}

void Window::addToTasklist(const QUrl &url, AppGroup *appGroup)
{
    int inserIndex = 0;
    int indexShow = m_listAppGroupShow.indexOf(appGroup);
    appGroup->setLocked(true);

    if (m_listAppGroupLocked.contains(appGroup))
    {
        // 本来就是锁定应用
        // 锁定应用调整位置
        QVariant values = SettingProcess::getValue(TASKBAR_LOCK_APP_KEY);
        QVariantList valuesList = values.toList();

        int newIndex = m_listAppGroupLocked.indexOf(appGroup);
        int oldIndex = valuesList.indexOf(url);

        valuesList.move(oldIndex, newIndex);

        KLOG_INFO() << "AppButtonContainer::addToTasklist move" << oldIndex << newIndex;

        SettingProcess::setValue(TASKBAR_LOCK_APP_KEY, valuesList);

        return;
    }
    else
    {
        // appGroup 不在 m_listAppGroupLocked 中

        // appGroup 在 m_listAppGroupShow 中
        // m_listAppGroupLocked 也在 m_listAppGroupShow 中
        // 判断 appGroup 在 m_listAppGroupLocked 中的位置

        // 无锁定应用，inserIndex = 0
        for (inserIndex = 0; inserIndex < m_listAppGroupLocked.size(); inserIndex++)
        {
            // 找锁定应用第一个大于显示位置的，就是要替换它的位置
            // 如果没找到，就是放在最后
            if (m_listAppGroupShow.indexOf(m_listAppGroupLocked.at(inserIndex)) > indexShow)
            {
                break;
            }
        }
    }

    QVariant values = SettingProcess::getValue(TASKBAR_LOCK_APP_KEY);
    QVariantList valuesList = values.toList();
    if (inserIndex >= valuesList.size())
    {
        valuesList.append(url);
        m_listAppGroupLocked.append(appGroup);
    }
    else
    {
        valuesList.insert(inserIndex, url);
        m_listAppGroupLocked.insert(inserIndex, appGroup);
    }

    KLOG_INFO() << "AppButtonContainer::addToTasklist" << inserIndex << valuesList.size();

    SettingProcess::setValue(TASKBAR_LOCK_APP_KEY, valuesList);
}

void Window::removeFromTasklist(const QUrl &url)
{
    SettingProcess::removeValueFromKey(TASKBAR_LOCK_APP_KEY, url);
}

void Window::removeGroup(AppGroup *group)
{
    auto iter = m_mapAppGroupOpened.begin();
    while (iter != m_mapAppGroupOpened.end())
    {
        if (iter.value() == group)
        {
            break;
        }
        iter++;
    }
    if (iter != m_mapAppGroupOpened.end())
    {
        m_mapAppGroupOpened.erase(iter);
    }

    m_listAppGroupLocked.removeAll(group);
    m_listAppGroupShow.removeAll(group);
    delete group;
    group = nullptr;

    updateLayout();
}

int Window::getInsertedIndex(const QPoint &pos)
{
#if 1
    // 在按钮区域：
    //      活动插入按钮：序号不变
    //      普通按钮：左右或上下 1/4 范围内，得到序号

    // 不在按钮区域：
    //      首尾区域，返回 0 或 最大值

    if (m_indicatorWidget->geometry().contains(pos))
    {
        return m_listAppGroupShow.indexOf(m_indicatorWidget);
    }

    if (m_listAppGroupShow.isEmpty())
    {
        return 0;
    }

    // 在按钮区域
    Qt::AlignmentFlag alignment = getLayoutAlignment();

    for (int i = 0; i < m_listAppGroupShow.size(); i++)
    {
        if (m_listAppGroupShow.at(i) == m_indicatorWidget)
        {
            continue;
        }
        QRect rect = m_listAppGroupShow.at(i)->geometry();
        if (rect.contains(pos))
        {
            QRect rectPrev;
            QRect rectNext;

            if (Qt::AlignLeft == alignment)
            {
                // 左右 1/4 判断
                rectPrev = rect.adjusted(0, 0, -rect.width() / 4 * 3, 0);
                rectNext = rect.adjusted(rect.width() / 4 * 3, 0, 0, 0);
            }
            else
            {
                // 上下 1/4 判断
                rectPrev = rect.adjusted(0, 0, 0, -rect.height() / 4 * 3);
                rectNext = rect.adjusted(0, rect.height() / 4 * 3, 0, 0);
            }

            if (rectPrev.contains(pos))
            {
                // 这里包含两种情况
                // 1. 本来就是当前位置的前一项
                //    则接触到马上变位置
                //    如果到next位置才变位置，则会导致一下变化太多
                // 2. 本来位于中间位置
                //    抢占位置i

                return i;
            }
            else if (rectNext.contains(pos))
            {
                // 这里包含两种情况
                // 1. 本来就是当前位置的下一项
                //    结果一致，i+1就是本来位置
                // 2. 本来位于中间位置
                //    抢占位置i+1

                return i + 1;
            }
            else
            {
                // 处于中间1/2处 打开文件
                return -1;
            }
        }
    }

    // 不在按钮区域
    if (Qt::AlignLeft == alignment)
    {
        if (pos.x() < m_listAppGroupShow.first()->pos().x())
        {
            return 0;
        }

        return m_listAppGroupShow.size();
    }
    else
    {
        if (pos.y() < m_listAppGroupShow.first()->pos().y())
        {
            return 0;
        }

        return m_listAppGroupShow.size();
    }
#else

    // 在按钮区域
    Qt::AlignmentFlag alignment = getLayoutAlignment();

    for (int i = 0; i < m_listAppGroupShow.size(); i++)
    {
        QRect rect = m_listAppGroupShow.at(i)->geometry();
        if (rect.contains(pos))
        {
            return i;
        }
    }

    // 不在按钮区域
    if (Qt::AlignLeft == alignment)
    {
        if (pos.x() < m_listAppGroupShow.first()->pos().x())
        {
            return 0;
        }

        return m_listAppGroupShow.size() - 1;
    }
    else
    {
        if (pos.y() < m_listAppGroupShow.first()->pos().y())
        {
            return 0;
        }

        return m_listAppGroupShow.size() - 1;
    }

#endif
}

int Window::getMovedIndex(AppGroup *appGroup)
{
    // 设正在移动的按钮组为a，正在检测是否让出位置的按钮组为b
    // a到达b一半位置时，调整移动项序号
    // 调整后，b刚好调整一个a的距离，保证a始终位于b的一侧

    // 还需要考虑分页情况

    QRect moveRect = appGroup->geometry();

    // 在按钮区域
    Qt::AlignmentFlag alignment = getLayoutAlignment();

    for (int i = 0; i < m_appPage[m_curPageIndex].size(); i++)
    {
        auto appGroup = m_appPage[m_curPageIndex].at(i);
        // 先计算其他的，如果找不到，再在循环外计算空白占位项
        if (appGroup == m_indicatorWidget)
        {
            continue;
        }

        QRect rect = appGroup->geometry();
        if (moveRect.intersected(rect).isNull())
        {
            // 不存在交值
            continue;
        }

        QRect rectPrev;
        QRect rectNext;
        if (Qt::AlignLeft == alignment)
        {
            // 左右 1/2
            rectPrev = rect.adjusted(0, 0, -rect.width() / 2, 0);
            rectNext = rect.adjusted(rect.width() / 2, 0, 0, 0);
        }
        else
        {
            // 上下 1/2
            rectPrev = rect.adjusted(0, 0, 0, -rect.height() / 2);
            rectNext = rect.adjusted(0, rect.height() / 2, 0, 0);
        }

        // 根据交值，判断是否两侧都有
        QRect intersectedRectPrev = rectPrev.intersected(moveRect);
        QRect intersectedRectNext = rectNext.intersected(moveRect);
        if (intersectedRectPrev.isNull() || intersectedRectNext.isNull())
        {
            // 只在一侧，不调整位置
            return m_listAppGroupShow.indexOf(m_indicatorWidget);
        }

        int areaPrev = intersectedRectPrev.width() * intersectedRectPrev.height();
        int areaNext = intersectedRectNext.width() * intersectedRectNext.height();
        if (areaPrev == areaNext)
        {
            // 只在一侧，不调整位置
            return m_listAppGroupShow.indexOf(m_indicatorWidget);
        }
        else
        {
            // KLOG_INFO() << "两侧均有交值，取代该位置" << i << QTime::currentTime() << moveRect << rect;
            // 两侧均有交值，取代该位置
            return m_listAppGroupShow.indexOf(appGroup);
        }
    }

    if (!m_indicatorWidget->geometry().intersected(moveRect).isNull())
    {
        // 仅与空白项有交值，不调整位置
        return m_listAppGroupShow.indexOf(m_indicatorWidget);
    }

    // 不在按钮区域
    QPoint pos = appGroup->geometry().center();
    if (Qt::AlignLeft == alignment)
    {
        if (pos.x() < m_appPage[m_curPageIndex].first()->pos().x())
        {
            return m_listAppGroupShow.indexOf(m_appPage[m_curPageIndex].first());
        }

        return m_listAppGroupShow.indexOf(m_appPage[m_curPageIndex].last());
    }
    else
    {
        if (pos.y() < m_listAppGroupShow.first()->pos().y())
        {
            return m_listAppGroupShow.indexOf(m_appPage[m_curPageIndex].first());
        }

        return m_listAppGroupShow.indexOf(m_appPage[m_curPageIndex].last());
    }
}

void Window::startMoveGroup(AppGroup *appGroup)
{
    int index = m_listAppGroupShow.indexOf(appGroup);
    if (-1 == index)
    {
        return;
    }
    m_listAppGroupShow.removeAll(appGroup);
    m_listAppGroupShow.insert(index, m_indicatorWidget);
    m_indicatorWidget->show();
    m_indicatorWidget->setFixedSize(appGroup->size());
    updateLayout(m_curPageIndex);
}

void Window::endMoveGroup(AppGroup *appGroup)
{
    if (m_listAppGroupShow.contains(appGroup))
    {
        return;
    }
    int index = m_listAppGroupShow.indexOf(m_indicatorWidget);
    m_listAppGroupShow.removeAll(m_indicatorWidget);
    m_listAppGroupShow.insert(index, appGroup);
    m_indicatorWidget->hide();
    auto size = m_import->getPanel()->getSize();
    m_indicatorWidget->setFixedSize(size, size);
    updateLayout(m_curPageIndex);
}

void Window::moveGroup(AppGroup *appGroup)
{
    int newIndex = getMovedIndex(appGroup);
    int oldIndex = m_listAppGroupShow.indexOf(m_indicatorWidget);
    if (newIndex != oldIndex)
    {
        // KLOG_INFO() << "moveGroup" << oldIndex << newIndex << m_listAppGroupShow.size();
        m_listAppGroupShow.move(oldIndex, newIndex);
        updateLayout(m_curPageIndex);
    }
}

Qt::AlignmentFlag Window::getLayoutAlignment()
{
    int orientation = m_import->getPanel()->getOrientation();
    Qt::AlignmentFlag alignment = (orientation == PanelOrientation::PANEL_ORIENTATION_BOTTOM ||
                                   orientation == PanelOrientation::PANEL_ORIENTATION_TOP)
                                      ? Qt::AlignLeft
                                      : Qt::AlignTop;

    return alignment;
}

QBoxLayout::Direction Window::getLayoutDirection()
{
    int orientation = m_import->getPanel()->getOrientation();
    auto direction = (orientation == PanelOrientation::PANEL_ORIENTATION_BOTTOM ||
                      orientation == PanelOrientation::PANEL_ORIENTATION_TOP)
                         ? QBoxLayout::Direction::LeftToRight
                         : QBoxLayout::Direction::TopToBottom;
    return direction;
}

void Window::openFileByDrop(QDropEvent *event)
{
    QByteArray mimeData = event->mimeData()->data("text/uri-list");
    if (!event->mimeData()->hasFormat("text/uri-list"))
    {
        return;
    }

    QList<QUrl> urls(event->mimeData()->urls());
    if (urls.isEmpty())
    {
        return;
    }

    QUrl appUrl;
    QPoint pos = event->pos();
    for (auto appGroup : m_listAppGroupShow)
    {
        if (appGroup->geometry().contains(pos))
        {
            appUrl = appGroup->getUrl();
        }
    }

    KService::Ptr service = KService::serviceByStorageId(appUrl.fileName());
    if (!service.data())
    {
        return;
    }

    auto *job = new KIO::ApplicationLauncherJob(service);
    job->setUrls(urls);
    job->start();

    //通知kactivitymanagerd
    KActivities::ResourceInstance::notifyAccessed(QUrl(QStringLiteral("applications:") + service->storageId()));
}

}  // namespace Taskbar

}  // namespace Kiran
