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
#include <qt5-log-i.h>
#include <KActivities/KActivities/ResourceInstance>
#include <KActivities/Stats/ResultSet>
#include <KActivities/Stats/ResultWatcher>
#include <KIO/ApplicationLauncherJob>
#include <KIOCore/KFileItem>
#include <KService/KService>
#include <QDragEnterEvent>
#include <QGSettings>
#include <QMimeData>
#include <QPainter>
#include <QTimer>
#include <algorithm>

#include "app-button.h"
#include "app-group.h"
#include "app-previewer.h"
#include "applet.h"
#include "ks-i.h"
#include "lib/common/logging-category.h"
#include "lib/common/utility.h"
#include "lib/common/window-info-helper.h"
#include "lib/common/window-manager.h"
#include "plugin-i.h"
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
      m_import(import)
{
    initUI();
    initConfig();

    updateLockApp();
    updateFavorite();

    initWindowManager();

    updateLayout();
}

Window::~Window() = default;

void Window::initUI()
{
    auto direction = getLayoutDirection();
    m_layout = new QBoxLayout(direction, this);
    m_layout->setMargin(appMargin);
    m_layout->setSpacing(appSpacing);
    setRadius(0);
    setAcceptDrops(true);

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
                showPageIndex = std::max(showPageIndex, 0);
                updateLayout(showPageIndex);
            });
    connect(m_downPageBtn, &QAbstractButton::clicked, [this]()
            {
                updateLayout(m_curPageIndex + 1);
            });

    // 拖动应用图标
    m_indicatorWidget = new AppGroup(m_import, this);
    m_indicatorWidget->hide();

    // 应用预览窗口
    m_appPreviewer = new AppPreviewer(m_import, this);
}

void Window::initWindowManager()
{
    connect((Applet *)parent(), &Applet::windowAdded, this, &Window::addWindow);
    connect((Applet *)parent(), &Applet::windowRemoved, this, &Window::removeWindow);
    connect((Applet *)parent(), &Applet::activeWindowChanged, [this](WId wid)
            {
                static WId lastWid = 0;
                if (lastWid != wid)
                {
                    lastWid = wid;
                    updateLayout();
                }
                emit activeWindowChanged(wid);
            });

    for (auto wid : WindowManagerInstance.getAllWindow())
    {
        addWindow(wid);
    }

    WId wid = WindowInfoHelper::activeWindow();
    emit activeWindowChanged(wid);
}

void Window::initConfig()
{
    m_gsettings = new QGSettings(TASKBAR_SCHEMA_ID, "", this);
    connect(m_gsettings, &QGSettings::changed, this, &Window::settingChanged);

    m_actStatsLinkedWatcher = new ResultWatcher(LinkedResources | Agent::global() | Type::any() | Activity::any(), this);
    connect(m_actStatsLinkedWatcher, &ResultWatcher::resultLinked, this, &Window::updateFavorite);
    connect(m_actStatsLinkedWatcher, &ResultWatcher::resultUnlinked, this, &Window::updateFavorite);

    auto *panelObject = dynamic_cast<QObject *>(m_import->getPanel());
    connect(panelObject, SIGNAL(panelProfileChanged()), this, SLOT(updateLayoutByProfile()));
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
    //    KLOG_INFO(LCTaskbar) << "dragLeaveEvent";
    m_currentDropIndex = -1;
    m_listAppGroupShow.removeAll(m_indicatorWidget);
    m_indicatorWidget->hide();
    updateLayout();

    event->accept();
}

void Window::dropEvent(QDropEvent *event)
{
    KLOG_INFO(LCTaskbar) << "dropEvent";

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

    QUrl appUrl;
    QList<QUrl> urls(event->mimeData()->urls());
    KLOG_INFO(LCTaskbar) << "dropEvent" << urls;
    for (const auto &url : urls)
    {
        KFileItem fileItem(url);
        if (fileItem.isNull())
        {
            continue;
        }

        appUrl = url;
        break;
    }

    if (appUrl.isEmpty())
    {
        event->accept();
        return;
    }

    AppInfo info(appUrl, {});
    AppGroup *appGroup = genAppGroup(info);

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
            for (auto *appGroup : m_listAppGroupShow)
            {
                if (m_listAppGroupLocked.contains(appGroup))
                {
                    listAppGroupLocked.append(appGroup);
                }
            }
            m_listAppGroupLocked = listAppGroupLocked;
            addToTasklist(appUrl, appGroup);
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

        addToTasklist(appUrl, appGroup);
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
    // 横竖摆放
    auto direction = getLayoutDirection();
    m_layout->setDirection(direction);
    // 子控件对齐方式：左右、上下
    Qt::AlignmentFlag alignment = getLayoutAlignment();
    m_layout->setAlignment(alignment);

    auto panelSize = m_import->getPanel()->getSize();
    setFixedDimensions(panelSize, direction);

    for (auto *appGroup : m_listAppGroupShow)
    {
        appGroup->updateLayout();
    }
    updateLayout();
}

AppGroup *Window::genAppGroup(const AppInfo &appInfo)
{
    AppGroup *appGroup = nullptr;

    // 是否是已固定应用
    for (auto *iter : m_listAppGroupLocked)
    {
        if (iter->getAppInfo() == appInfo)
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
        if (iter.first->getAppInfo() == appInfo)
        {
            appGroup = iter.first;
            break;
        }
    }

    if (appGroup)
    {
        return appGroup;
    }

    KLOG_INFO(LCTaskbar) << "new AppGroup:" << appInfo;

    appGroup = new AppGroup(m_import, appInfo, this);

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
    AppInfo appInfo;
    if (!getAppInfo(wid, appInfo))
    {
        return;
    }

    KLOG_INFO(LCTaskbar) << "addWindow" << wid << appInfo;

    // 锁定应用的打开
    AppGroup *appGroup = nullptr;
    if (!appInfo.m_url.isEmpty())
    {
        for (auto *appLocked : m_listAppGroupLocked)
        {
            if (appLocked->getAppInfo() == appInfo)
            {
                appGroup = appLocked;
                break;
            }
        }
    }

    if (!appGroup)
    {
        if (!m_mapAppGroupOpened.contains(appInfo))
        {
            // 创建组
            appGroup = genAppGroup(appInfo);
        }
        else
        {
            appGroup = m_mapAppGroupOpened[appInfo].first;
        }
    }

    if (m_mapAppGroupOpened.contains(appInfo))
    {
        m_mapAppGroupOpened[appInfo].second.append(wid);
    }
    else
    {
        m_mapAppGroupOpened[appInfo] = qMakePair(appGroup, QList<WId>({wid}));
    }

    if (!m_listAppGroupShow.contains(appGroup))
    {
        m_listAppGroupShow.append(appGroup);
    }

    appGroup->addWindow(wid);
    m_appPreviewer->addWindow(wid);

    updateLayout();
}

void Window::removeWindow(WId wid)
{
    KLOG_INFO(LCTaskbar) << "removeWindow" << wid;

    auto iter = m_mapAppGroupOpened.begin();
    while (iter != m_mapAppGroupOpened.end())
    {
        if (iter.value().second.contains(wid))
        {
            break;
        }
        iter++;
    }
    if (iter != m_mapAppGroupOpened.end())
    {
        auto info = iter.key();
        auto *group = iter.value().first;
        auto widList = iter.value().second;
        group->removeWindow(wid);
        widList.removeAll(wid);
        if (widList.isEmpty())
        {
            m_mapAppGroupOpened.remove(info);
        }

        m_appPreviewer->removeWindow(wid);
    }
}

bool Window::getAppInfo(WId wid, AppInfo &info)
{
    QUrl url = WindowInfoHelper::getUrlByWId(wid);
    QByteArray wmClass = WindowInfoHelper::getWmClassByWId(wid);
    if (url.isEmpty() && wmClass.isEmpty())
    {
        KLOG_WARNING(LCTaskbar) << "can't find url and wmclass by wid:" << wid;
        return false;
    }

    info = AppInfo(url, wmClass);
    return true;
}

void Window::updateLayout(int showPageIndex)
{
    Utility::clearLayout(m_layout, false, false);

    // 横竖摆放
    auto direction = getLayoutDirection();
    // 子控件对齐方式：左右、上下
    Qt::AlignmentFlag alignment = getLayoutAlignment();

    // 任务栏空间总大小
    int totalSize = direction == QBoxLayout::Direction::LeftToRight ? width() : height();

    int appSizeCount = appMargin * 2;  // 累计app宽或高

    m_appPage.clear();
    m_appPage.push_back(QList<AppGroup *>());  // 新建空页

    for (auto *appGroup : m_listAppGroupShow)
    {
        int addSize = 0;
        adjustAndGetSize(appGroup, direction, addSize);

        if (shouldCreateNewPage(appGroup, appSizeCount, addSize, totalSize, direction))
        {
            m_appPage.push_back(QList<AppGroup *>());  // 超出页面，新建空页
            appSizeCount = appMargin * 2;              // 重置累计值
        }

        appSizeCount += addSize;
        m_appPage.last().push_back(appGroup);
    }

    calculateCurrentPageIndex(showPageIndex);

    // 显示对应页
    for (auto *appGroup : m_appPage[m_curPageIndex])
    {
        appGroup->show();
        m_layout->addWidget(appGroup);
    }

    // 其他应用隐藏
    for (auto *appGroup : m_listAppGroupShow)
    {
        if (!m_appPage[m_curPageIndex].contains(appGroup))
        {
            appGroup->hide();
        }
    }

    // 翻页按钮
    updatePageButtons(direction, alignment);

    // 强制重布局
    // 解决鼠标拖动过程中，鼠标偏离控件，会导致布局不能及时更新
    m_layout->activate();
}

void Window::setFixedDimensions(int size, QBoxLayout::Direction direction)
{
    if (QBoxLayout::Direction::LeftToRight == direction)
    {
        setMaximumWidth(QWIDGETSIZE_MAX);
        setFixedHeight(size);
    }
    else
    {
        setMaximumHeight(QWIDGETSIZE_MAX);
        setFixedWidth(size);
    }
}

void Window::adjustAndGetSize(AppGroup *appGroup, QBoxLayout::Direction direction, int &addSize)
{
    // 为什么先调整大小?
    // 1.AppButton大小变化 2.当前窗口缩放时,引起AppGroup缩放
    // 以上缩放不会马上应用到实际大小,导致获取大小有偏差
    appGroup->adjustSize();
    appGroup->updateGeometry();  // 确保尺寸更新完成

    if (direction == QBoxLayout::Direction::LeftToRight)
    {
        addSize = appGroup->width();
    }
    else
    {
        addSize = appGroup->height();
    }

    addSize += appSpacing;

    //    KLOG_INFO() << "adjustAndGetSize:" << direction << addSize << appGroup->getAppInfo() << appGroup;
}

bool Window::shouldCreateNewPage(AppGroup *appGroup, int appSizeCount, int addSize, int totalSize, QBoxLayout::Direction direction)
{
    // 如果是最后一个app,不需要提前将翻页按钮大小计算进去,可能不需要显示翻页按钮
    if (m_listAppGroupShow.last() == appGroup)
    {
        // KLOG_INFO() << "预计需要空间" << appSizeCount + addSize << "原始空间" << totalSize;

        return appSizeCount + addSize > totalSize;
    }
    // KLOG_INFO() << "预计需要空间1" << appSizeCount + addSize + m_upPageBtn->width() * 2 << "原始空间" << totalSize;
    return appSizeCount + addSize + m_upPageBtn->width() * 2 > totalSize;
}

void Window::calculateCurrentPageIndex(int showPageIndex)
{
    // 计算当前页面序号
    // 获取当前激活窗口信息
    // 判断位于哪页，就是当前页面序号
    if (showPageIndex < 0 || showPageIndex > m_appPage.size())
    {
        // 未指定显示哪页,按照窗口激活情况进行显示
        if (1 == m_appPage.size())
        {
            m_curPageIndex = 0;
            return;
        }

        auto wid = WindowInfoHelper::activeWindow();
        if (0 == wid)
        {
            m_curPageIndex = 0;
            return;
        }

        auto url = WindowInfoHelper::getUrlByWId(wid);
        auto wmClass = WindowInfoHelper::getWmClassByWId(wid);
        AppInfo info = AppInfo(url, wmClass);
        if (!m_mapAppGroupOpened.contains(info))
        {
            m_curPageIndex = 0;
        }
        else
        {
            auto *appGroup = m_mapAppGroupOpened[info].first;
            for (m_curPageIndex = 0; m_curPageIndex < m_appPage.size(); ++m_curPageIndex)
            {
                if (m_appPage[m_curPageIndex].contains(appGroup))
                {
                    break;
                }
            }
        }
    }
    else
    {
        m_curPageIndex = std::min(showPageIndex, m_appPage.size() - 1);
    }
}

void Window::updatePageButtons(QBoxLayout::Direction direction, Qt::AlignmentFlag alignment)
{
    if (m_appPage.size() > 1)
    {
        auto *layoutPage = new QBoxLayout(direction);
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
}

void Window::updateLockApp()
{
    QVariantList appUrls = m_gsettings->get(TASKBAR_SCHEMA_KEY_FIXED_APPS).toList();
    for (auto *appGroup : m_listAppGroupLocked)
    {
        auto info = appGroup->getAppInfo();
        if (!appUrls.contains(info.m_url))
        {
            removeLockApp(info);
        }
    }
    for (const auto &appUrl : appUrls)
    {
        addLockApp(appUrl.toUrl());
    }
}

void Window::addLockApp(const QUrl &url)
{
    AppInfo info(url, {});
    AppGroup *appGroup = genAppGroup(info);
    if (!appGroup)
    {
        return;
    }
    appGroup->setLocked(true);

    if (!m_listAppGroupLocked.contains(appGroup))
    {
        m_listAppGroupLocked.append(appGroup);
    }

    if (!m_listAppGroupShow.contains(appGroup))
    {
        // 若没有打开，则往前排
        // if (!appGroup->isOpened())
        // {
        //     m_listAppGroupShow.insert(0, appGroup);
        // }
        // else
        {
            m_listAppGroupShow.append(appGroup);
        }
    }
}

void Window::removeLockApp(const AppInfo &info)
{
    AppGroup *appGroup = nullptr;
    for (auto *app : m_listAppGroupLocked)
    {
        if (app->getAppInfo() == info)
        {
            appGroup = app;
            break;
        }
    }

    if (appGroup)
    {
        m_listAppGroupLocked.removeAll(appGroup);
        appGroup->setLocked(false);

        // 如果固定的应用没有被打开，则移除掉应用
        auto iter = m_mapAppGroupOpened.begin();
        while (iter != m_mapAppGroupOpened.end())
        {
            if (iter.value().first == appGroup)
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
        KLOG_WARNING(LCTaskbar) << "can't find lock app:appId";
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
    KLOG_INFO(LCTaskbar) << "addToFavorite" << appIdReal;
    m_actStatsLinkedWatcher->linkToActivity(QUrl(appIdReal), Activity::global(), Agent::global());
}

void Window::removeFromFavorite(const QString &appId)
{
    QString appIdReal = QLatin1String("applications:") + appId;
    m_actStatsLinkedWatcher->unlinkFromActivity(QUrl(appIdReal), Activity::global(), Agent::global());
}

void Window::isInTasklist(const QUrl &url, bool &checkResult)
{
    QVariantList appUrls = m_gsettings->get(TASKBAR_SCHEMA_KEY_FIXED_APPS).toList();
    checkResult = appUrls.contains(url);
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
        QVariantList valuesList = m_gsettings->get(TASKBAR_SCHEMA_KEY_FIXED_APPS).toList();

        int newIndex = m_listAppGroupLocked.indexOf(appGroup);
        int oldIndex = valuesList.indexOf(url);
        valuesList.move(oldIndex, newIndex);
        KLOG_INFO(LCTaskbar) << "addToTasklist move" << oldIndex << newIndex;
        m_gsettings->set(TASKBAR_SCHEMA_KEY_FIXED_APPS, valuesList);

        return;
    }

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

    QVariantList valuesList = m_gsettings->get(TASKBAR_SCHEMA_KEY_FIXED_APPS).toList();
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
    KLOG_INFO(LCTaskbar) << "addToTasklist" << inserIndex << valuesList.size();
    m_gsettings->set(TASKBAR_SCHEMA_KEY_FIXED_APPS, valuesList);
}

void Window::removeFromTasklist(const QUrl &url)
{
    QVariantList valuesList = m_gsettings->get(TASKBAR_SCHEMA_KEY_FIXED_APPS).toList();
    valuesList.removeAll(url);
    m_gsettings->set(TASKBAR_SCHEMA_KEY_FIXED_APPS, valuesList);
}

void Window::removeGroup(AppGroup *group)
{
    // 移除应用组
    auto iter = m_mapAppGroupOpened.begin();
    while (iter != m_mapAppGroupOpened.end())
    {
        if (iter.value().first == group)
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
    // 在按钮区域：
    //      活动插入按钮：序号不变
    //      普通按钮：左右或上下 1/4 范围内，得到序号

    // 不在按钮区域：
    //      首尾区域，返回 0 或 最大值
    //      在两个按钮的中间，返回中间的序号
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

            if (rectNext.contains(pos))
            {
                // 这里包含两种情况
                // 1. 本来就是当前位置的下一项
                //    结果一致，i+1就是本来位置
                // 2. 本来位于中间位置
                //    抢占位置i+1

                return i + 1;
            }

            // 处于中间1/2处 打开文件
            return -1;
        }
    }

    // 不在按钮区域
    // 横向
    if (Qt::AlignLeft == alignment)
    {
        for (size_t i = 0; i < m_listAppGroupShow.size(); i++)
        {
            if (pos.x() < m_listAppGroupShow.at(i)->pos().x())
            {
                return i;
            }
        }

        return m_listAppGroupShow.size();
    }
    
    // 纵向
    for (size_t i = 0; i < m_listAppGroupShow.size(); i++)
    {
        if (pos.y() < m_listAppGroupShow.first()->pos().y())
        {
            return i;
        }
    }
    return m_listAppGroupShow.size();
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

    for (auto *appGroup : m_appPage[m_curPageIndex])
    {
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

        // KLOG_INFO(LCTaskbar) << "两侧均有交值，取代该位置" << i << QTime::currentTime() << moveRect << rect;
        // 两侧均有交值，取代该位置
        return m_listAppGroupShow.indexOf(appGroup);
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

    if (pos.y() < m_listAppGroupShow.first()->pos().y())
    {
        return m_listAppGroupShow.indexOf(m_appPage[m_curPageIndex].first());
    }

    return m_listAppGroupShow.indexOf(m_appPage[m_curPageIndex].last());
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
        // KLOG_INFO(LCTaskbar) << "moveGroup" << oldIndex << newIndex << m_listAppGroupShow.size();
        m_listAppGroupShow.move(oldIndex, newIndex);
        updateLayout(m_curPageIndex);
    }
}

void Window::settingChanged(const QString &key)
{
    if (TASKBAR_SCHEMA_KEY_FIXED_APPS == key)
    {
        for (auto *appGroup : m_listAppGroupShow)
        {
            appGroup->updateLayout();
        }
        updateLockApp();
        updateLayout();
    }

    if (TASKBAR_SCHEMA_KEY_SHOW_APP_NAME == key)
    {
        for (auto *appGroup : m_listAppGroupShow)
        {
            appGroup->updateLayout();
        }
        updateLayout();
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
    for (auto *appGroup : m_listAppGroupShow)
    {
        if (appGroup->geometry().contains(pos))
        {
            appUrl = appGroup->getAppInfo().m_url;
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

    // 通知kactivitymanagerd
    KActivities::ResourceInstance::notifyAccessed(QUrl(QStringLiteral("applications:") + service->storageId()));
}

}  // namespace Taskbar

}  // namespace Kiran
