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
#include <kiran-style/style-palette.h>
#include <ks-i.h>
#include <plugin-i.h>
#include <qt5-log-i.h>
#include <KActivities/Stats/ResultSet>
#include <KActivities/Stats/ResultWatcher>
#include <KIOCore/KFileItem>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QPainter>
#include <QSettings>
#include <QTimer>

#include "app-button-container.h"
#include "app-button.h"
#include "app-group.h"
#include "applet.h"
#include "lib/common/define.h"
#include "lib/common/setting-process.h"
#include "lib/common/utility.h"
#include "lib/common/window-info-helper.h"

namespace KAStats = KActivities::Stats;
using namespace KAStats;
using namespace KAStats::Terms;

namespace Kiran
{
namespace Taskbar
{
AppButtonContainer::AppButtonContainer(IAppletImport *import, Applet *parent)
    : KiranColorBlock(parent),
      m_import(import),
      m_indicatorWidget(nullptr)
{
    auto direction = getLayoutDirection();
    m_layout = new QBoxLayout(direction, this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    setLayout(m_layout);

    QObject *Object = dynamic_cast<QObject *>(m_import->getPanel());
    connect(Object, SIGNAL(panelProfileChanged()), this, SLOT(updateLayout()));

    updateLockApp();

    QString settingDir = QFileInfo(KIRAN_SHELL_SETTING_FILE).dir().path();
    if (!QDir(settingDir).exists())
    {
        QDir().mkpath(settingDir);
    }
    // QSettings 保存时，会删除原有文件，重新创建一个新文件，所以不能监视文件，此处监视文件夹
    m_settingFileWatcher.addPath(settingDir);
    connect(&m_settingFileWatcher, &QFileSystemWatcher::directoryChanged, this, [=]()
            { updateLockApp(); });

    m_actStatsLinkedWatcher = new ResultWatcher(LinkedResources | Agent::global() | Type::any() | Activity::any(), this);
    connect(m_actStatsLinkedWatcher, &ResultWatcher::resultLinked, this, &AppButtonContainer::updateFavorite);
    connect(m_actStatsLinkedWatcher, &ResultWatcher::resultUnlinked, this, &AppButtonContainer::updateFavorite);
    updateFavorite();

    connect(parent, &Applet::windowAdded, this, &AppButtonContainer::addWindow);
    connect(parent, &Applet::windowRemoved, this, &AppButtonContainer::windowRemoved);
    connect(parent, &Applet::activeWindowChanged, this, &AppButtonContainer::activeWindowChanged);

    // 等待应用加载
    QTimer::singleShot(1000, this, [this]()
                       {
                           WId wid = WindowInfoHelper::activeWindow();
                           emit activeWindowChanged(wid);
                       });

    setAcceptDrops(true);
    m_currentDropIndex = -1;
    m_indicatorWidget = new AppGroup(m_import, this);
    m_indicatorWidget->hide();

    updateLayout();
}

AppButtonContainer::~AppButtonContainer()
{
}

void AppButtonContainer::dragEnterEvent(QDragEnterEvent *event)
{
    for (auto fmt : event->mimeData()->formats())
    {
        KLOG_INFO() << fmt << event->mimeData()->data(fmt);
    }
    m_currentDropIndex = 0;
    QByteArray mimeData = event->mimeData()->data("text/uri-list");
    if (event->mimeData()->hasFormat("text/uri-list"))
    {
        QList<QUrl> urls(event->mimeData()->urls());
        for (auto url : urls)
        {
            m_indicatorWidget->setDragData(url);
            event->accept();
            return;
        }
    }
    else
    {
        event->ignore();
    }
}

void AppButtonContainer::dragMoveEvent(QDragMoveEvent *event)
{
    QPoint pos = event->pos();
    int index = getInsertedIndex(pos);
    if (-1 == index)
    {
        m_listAppGroupShow.removeAll(m_indicatorWidget);
        m_indicatorWidget->hide();
        updateLayout();
        event->ignore();
        return;
    }

    m_currentDropIndex = index;

    if (m_currentDropIndex != m_listAppGroupShow.indexOf(m_indicatorWidget))
    {
        m_listAppGroupShow.removeAll(m_indicatorWidget);

        // 直接使用insert，效果一样，但是Qt有警告
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

void AppButtonContainer::dragLeaveEvent(QDragLeaveEvent *event)
{
    m_currentDropIndex = 0;
    m_listAppGroupShow.removeAll(m_indicatorWidget);
    m_indicatorWidget->hide();
    updateLayout();

    event->accept();
}

void AppButtonContainer::dropEvent(QDropEvent *event)
{
    QByteArray mimeData = event->mimeData()->data("text/uri-list");
    if (!event->mimeData()->hasFormat("text/uri-list"))
    {
        QWidget::dropEvent(event);
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
            event->ignore();
            continue;
        }

        appBaseinfo.m_url = url;
        break;
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
    //    QWidget::dropEvent(event);
}

AppGroup *AppButtonContainer::genAppGroup(const AppBaseInfo &baseinfo)
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

    connect(appGroup, &AppGroup::isInFavorite, this, &AppButtonContainer::isInFavorite, Qt::DirectConnection);
    connect(appGroup, &AppGroup::isInTasklist, this, &AppButtonContainer::isInTasklist, Qt::DirectConnection);
    connect(appGroup, &AppGroup::addToFavorite, this, &AppButtonContainer::addToFavorite);
    connect(appGroup, &AppGroup::removeFromFavorite, this, &AppButtonContainer::removeFromFavorite);
    connect(appGroup, &AppGroup::addToTasklist, this, &AppButtonContainer::addToTasklist);
    connect(appGroup, &AppGroup::removeFromTasklist, this, &AppButtonContainer::removeFromTasklist);
    connect(appGroup, &AppGroup::emptyGroup, this, &AppButtonContainer::removeGroup);

    return appGroup;
}

void AppButtonContainer::addWindow(WId wid)
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

void AppButtonContainer::updateLayout()
{
    Utility::clearLayout(m_layout, false, false);

    //横竖摆放
    auto direction = getLayoutDirection();
    m_layout->setDirection(direction);
    //子控件对齐方式：左右、上下
    Qt::AlignmentFlag alignment = getLayoutAlignment();
    m_layout->setAlignment(alignment);

    if (QBoxLayout::Direction::LeftToRight == direction)
    {
        m_layout->setContentsMargins(10, 0, 10, 0);
    }
    else
    {
        m_layout->setContentsMargins(0, 10, 0, 10);
    }
    //    KLOG_INFO() << "m_listAppGroupShow" << m_listAppGroupShow << m_indicatorWidget;
    for (auto appGroup : m_listAppGroupShow)
    {
        m_layout->addWidget(appGroup);
    }
}

void AppButtonContainer::updateLockApp()
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

    updateLayout();
}

void AppButtonContainer::addLockApp(const QUrl &url)
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

void AppButtonContainer::removeLockApp(const QUrl &url)
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

void AppButtonContainer::updateFavorite()
{
    m_favoriteAppId.clear();

    const auto query = LinkedResources | Agent::global() | Type::any() | Activity::any();
    for (const ResultSet::Result &result : ResultSet(query))
    {
        QString serviceId = QUrl(result.resource()).path();
        m_favoriteAppId.append(serviceId);
    }
}

void AppButtonContainer::isInFavorite(const QString &appId, bool &checkResult)
{
    checkResult = m_favoriteAppId.contains(appId);
}

void AppButtonContainer::addToFavorite(const QString &appId)
{
    QString appIdReal = QLatin1String("applications:") + appId;
    KLOG_WARNING() << "addToFavorite" << appIdReal;
    m_actStatsLinkedWatcher->linkToActivity(QUrl(appIdReal), Activity::global(), Agent::global());
}

void AppButtonContainer::removeFromFavorite(const QString &appId)
{
    QString appIdReal = QLatin1String("applications:") + appId;
    m_actStatsLinkedWatcher->unlinkFromActivity(QUrl(appIdReal), Activity::global(), Agent::global());
}

void AppButtonContainer::isInTasklist(const QUrl &url, bool &checkResult)
{
    checkResult = SettingProcess::isValueInKey(TASKBAR_LOCK_APP_KEY, url);
}

void AppButtonContainer::addToTasklist(const QUrl &url, AppGroup *appGroup)
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

void AppButtonContainer::removeFromTasklist(const QUrl &url)
{
    SettingProcess::removeValueFromKey(TASKBAR_LOCK_APP_KEY, url);
}

void AppButtonContainer::removeGroup(AppGroup *group)
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

int AppButtonContainer::getInsertedIndex(const QPoint &pos)
{
    // 在按钮区域：
    //      活动插入按钮：序号不变
    //      普通按钮：左右或上下 1/4 范围内，得到序号

    // 不在按钮区域：
    //      首尾区域，返回 0 或 最大值

    // 在按钮区域
    if (m_indicatorWidget->geometry().contains(pos))
    {
        return m_currentDropIndex;
    }

    Qt::AlignmentFlag alignment = getLayoutAlignment();

    for (int i = 0; i < m_listAppGroupShow.size(); i++)
    {
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
                return i;
            }
            else if (rectNext.contains(pos))
            {
                return i + 1;
            }
            else
            {
                return -1;
            }
        }
    }

    // 不在按钮区域
    if (m_listAppGroupShow.isEmpty())
    {
        return 0;
    }

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
}

Qt::AlignmentFlag AppButtonContainer::getLayoutAlignment()
{
    int orientation = m_import->getPanel()->getOrientation();
    Qt::AlignmentFlag alignment = (orientation == PanelOrientation::PANEL_ORIENTATION_BOTTOM ||
                                   orientation == PanelOrientation::PANEL_ORIENTATION_TOP)
                                      ? Qt::AlignLeft
                                      : Qt::AlignTop;

    return alignment;
}

QBoxLayout::Direction AppButtonContainer::getLayoutDirection()
{
    int orientation = m_import->getPanel()->getOrientation();
    auto direction = (orientation == PanelOrientation::PANEL_ORIENTATION_BOTTOM ||
                      orientation == PanelOrientation::PANEL_ORIENTATION_TOP)
                         ? QBoxLayout::Direction::LeftToRight
                         : QBoxLayout::Direction::TopToBottom;
    return direction;
}

}  // namespace Taskbar

}  // namespace Kiran
