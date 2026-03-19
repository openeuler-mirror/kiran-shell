/**
 * Copyright (c) 2020 ~ 2025 KylinSec Co., Ltd.
 * kiran-shell is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     liuxinhao <liuxinhao@kylinsec.com.cn>
 */
#include "apps-model.h"
#include <qt5-log-i.h>
#include <KService/KServiceGroup>
#include <KService/KSycoca>
#include <QDebug>
#include <QGSettings>
#include <QIcon>
#include <QThread>
#include <QTimer>
#include "apps-model-private.h"
#include "lib/common/utility.h"
#include "new-apps-manager.h"

namespace Kiran
{
namespace Menu
{
static bool isChineseLocale()
{
    QLocale locale;
    // 判断语言是否为中文
    return locale.language() == QLocale::Chinese;
}

static constexpr int kMaxIconRetryCount = 3;
static constexpr int kIconRetryIntervalsMs[kMaxIconRetryCount] = {500, 1500, 3000};

AppsModel::AppsModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    // 初始化DataLoader并移动至工作线程
    m_dataLoadThread = new QThread(this);
    m_dataLoadThread->setObjectName("AppsDataLoaderThread");

    m_dataLoader = new AppsDataLoader();
    m_dataLoader->moveToThread(m_dataLoadThread);

    connect(m_dataLoadThread, &QThread::started, m_dataLoader, &AppsDataLoader::loadData);
    connect(m_dataLoader, &AppsDataLoader::dataLoaded, this, &AppsModel::onDataLoaded);
    connect(m_dataLoader, &AppsDataLoader::errorOccurred, this, &AppsModel::onErrorOccurred);
    connect(&NewAppsManager::getInstance(), &NewAppsManager::newAppsConfigUpdated, m_dataLoader, &AppsDataLoader::loadData);

    m_dataLoadThread->start();
}

AppsModel::~AppsModel()
{
    m_dataLoadThread->requestInterruption();
    m_dataLoadThread->quit();
    if (!m_dataLoadThread->wait(100))
    {
        KLOG_WARNING(LCMenu) << "AppsDataLoaderThread did not exit in time, terminating it.";
        m_dataLoadThread->terminate();
        m_dataLoadThread->wait();
    }
    delete m_dataLoader;
    m_dataLoader = nullptr;
    delete m_dataLoadThread;
    m_dataLoadThread = nullptr;
    AppNode::cleanup(m_root);
    AppNode::cleanup(m_searchRoot);
}

void AppsModel::setFilterText(const QString &text)
{
    if (text.isEmpty())
    {
        // 清空搜索项，通知视图模型将重建
        beginResetModel();
        if (m_searchRoot != nullptr)
        {
            AppNode::cleanup(m_searchRoot);
            m_searchRoot = nullptr;
        }
        endResetModel();
        emit dataChanged(index(0, 0), index(rowCount() - 1, 0));
        return;
    }
    else
    {
        // 替换搜索结果，释放旧的搜索根项
        QSet<QString> loadedAppStorageIds;

        auto searchRoot = AppNode::createCategory("SearchRoot");
        auto searchResultItem = AppNode::createCategory(tr("Search Result"), searchRoot);
        filter(searchResultItem, text, m_root, loadedAppStorageIds);

        beginResetModel();
        if (m_searchRoot)
        {
            AppNode::cleanup(m_searchRoot);
        }
        m_searchRoot = searchRoot;
        endResetModel();

        emit dataChanged(index(0, 0), index(rowCount() - 1, 0));
    }
}

QString AppsModel::getID(const QModelIndex &index)
{
    auto var = data(index, AppsModel::IdRole);
    return var.toString();
}

bool AppsModel::isCategory(const QModelIndex &index)
{
    auto var = data(index, AppsModel::TypeRole);
    return var.toBool();
}

QIcon AppsModel::getIcon(const QModelIndex &index)
{
    return qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
}

// 基于原始数据树进行过键值过滤，检查应用是否匹配，新形成一个数据树
void AppsModel::filter(AppNode *searchResult, const QString &searchKey, AppNode *item, QSet<QString> &storageIds)
{
    if (!item)
        return;

    for (int i = 0; i < item->childCount(); ++i)
    {
        AppNode *child = item->child(i);
        if (child->data(AppsModel::TypeRole).toInt() != AppsModel::ItemType::Application)
        {
            // 如果是分类，递归搜索子分类
            filter(searchResult, searchKey, child, storageIds);
            continue;
        }

        // 如果是应用，检查是否匹配
        bool match = false;
        QString appName = child->data(AppsModel::NameRole).toString();
        QString untranslatedName = child->data(AppsModel::UntranslatedNameRole).toString();
        QString appId = child->data(AppsModel::IdRole).toString();
        if (appName.contains(searchKey, Qt::CaseInsensitive))
        {
            match = true;
        }
        else if (untranslatedName.contains(searchKey, Qt::CaseInsensitive))
        {
            match = true;
        }
        else if (isChineseLocale())
        {
            auto pinyinGuess = Utility::pinyinGuess(searchKey);
            for (auto pinyinEntry : pinyinGuess)
            {
                if (appName.contains(pinyinEntry, Qt::CaseInsensitive))
                {
                    match = true;
                    break;
                }
            }
        }

        // 不添加重复搜索项,新应用会存在于两个节点下
        if (match & !storageIds.contains(appId))
        {
            AppNode::createNode(child->data(), searchResult);
            storageIds << appId;
        }
    }
}

void AppsModel::buildNewAppsSubTree(QSet<QString> &newAppsSet, AppNode *parent, AppNode *newAppsNode)
{
    if (!parent || newAppsSet.isEmpty())
        return;

    // 遍历子节点，查找新应用
    for (int i = 0; i < parent->childCount(); ++i)
    {
        AppNode *child = parent->child(i);
        if (child->data(AppsModel::TypeRole).toInt() == AppsModel::ItemType::Application)
        {
            QString appId = child->data(AppsModel::IdRole).toString();
            if (newAppsSet.contains(appId))
            {
                AppNode::createNode(child->data(), newAppsNode);
                newAppsSet.remove(appId);
            }
        }
        else
        {
            // 如果是分类，递归处理子分类
            buildNewAppsSubTree(newAppsSet, child, newAppsNode);
        }
    }

    return;
}

void AppsModel::collectFallbackIconApps(AppNode *parent, const QSet<QString> &targetAppIds, QSet<QString> &fallbackAppIds) const
{
    if (!parent || targetAppIds.isEmpty())
    {
        return;
    }

    for (int i = 0; i < parent->childCount(); ++i)
    {
        AppNode *child = parent->child(i);
        if (child->data(AppsModel::TypeRole).toInt() == AppsModel::ItemType::Application)
        {
            const auto appId = child->data(AppsModel::IdRole).toString();
            // 只跟踪目标集合内且当前仍是回退图标的应用。
            if (targetAppIds.contains(appId) && child->data(AppsModel::FallbackIconRole).toBool())
            {
                fallbackAppIds.insert(appId);
            }
            continue;
        }

        collectFallbackIconApps(child, targetAppIds, fallbackAppIds);
    }
}

void AppsModel::scheduleIconRetry()
{
    // 仅在存在待修复应用且仍有重试预算时触发。
    if (m_iconRetryScheduled || m_pendingIconRetryAppIds.isEmpty() || m_remainingIconRetryCount <= 0)
    {
        return;
    }

    const int retryIndex = kMaxIconRetryCount - m_remainingIconRetryCount;
    const int delayMs = kIconRetryIntervalsMs[retryIndex];
    --m_remainingIconRetryCount;
    m_iconRetryScheduled = true;

    QTimer::singleShot(delayMs, this, [this]()
                       {
                           m_iconRetryScheduled = false;
                           // 在延迟窗口中若图标已恢复，则不再触发额外重载。
                           if (m_pendingIconRetryAppIds.isEmpty())
                           {
                               return;
                           }

                           QMetaObject::invokeMethod(m_dataLoader, "loadData", Qt::QueuedConnection);
                       });
}

void AppsModel::updateIconRetryState(AppNode *appsRoot, const QSet<QString> &newAppIds)
{
    // 需要检测的集合 = 历史未修复应用 + 本次新增应用。
    QSet<QString> targetAppIds = m_pendingIconRetryAppIds;
    targetAppIds.unite(newAppIds);
    if (targetAppIds.isEmpty())
    {
        return;
    }

    QSet<QString> fallbackAppIds;
    collectFallbackIconApps(appsRoot, targetAppIds, fallbackAppIds);
    // 只保留“当前仍命中回退图标”的应用，避免无效重试。
    m_pendingIconRetryAppIds = fallbackAppIds;

    if (fallbackAppIds.isEmpty())
    {
        m_remainingIconRetryCount = 0;
        return;
    }

    QSet<QString> newFallbackApps = fallbackAppIds & newAppIds;
    if (!newFallbackApps.isEmpty())
    {
        // 新出现回退图标时重置重试预算，保证最多 3 次机会。
        m_remainingIconRetryCount = kMaxIconRetryCount;
    }

    scheduleIconRetry();
}

void AppsModel::onDataLoaded(QSet<QString> appIds, AppNode *appsRoot)
{
    static bool firstInit = true;
    auto& newAppsManager = NewAppsManager::getInstance();
    QSet<QString> diffApps;

    if (!firstInit)
    {
        // 检测新老数据差集
        diffApps = appIds - m_appIds;
        KLOG_INFO(LCMenu) << appIds << m_appIds;
        if (!diffApps.isEmpty())
        {
            newAppsManager.updateNewApps(diffApps);
        }
    }
    firstInit = false;

    updateIconRetryState(appsRoot, diffApps);

    // 剔除一些GSettings新应用配置中的无效项
    auto newAppsSet = newAppsManager.getAllNewApps();
    if (!newAppsSet.isEmpty())
    {
        // 仅保留当前在应用列表中的新应用
        newAppsSet = newAppsSet & appIds;
        newAppsManager.updateNewApps(newAppsSet);
    }

    // 根据最新的应用树，拷贝构建新的应用子树
    if (!newAppsSet.isEmpty())
    {
        KLOG_INFO(LCMenu) << "found new apps:" << newAppsSet.values().join(",");
        auto newAppsTreeNode = AppNode::createCategory(tr("New Apps"));
        buildNewAppsSubTree(newAppsSet, appsRoot, newAppsTreeNode);
        appsRoot->prepend(newAppsTreeNode);
    }

    // 替换原始数据源
    beginResetModel();
    m_appIds = appIds;
    if (m_root)
    {
        AppNode::cleanup(m_root);
    }
    m_root = appsRoot;
    endResetModel();

    emit dataChanged(index(0, 0), index(rowCount() - 1, 0));
}

void AppsModel::onErrorOccurred()
{
}

QVariant AppsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    AppNode *item = static_cast<AppNode *>(index.internalPointer());
    if (!item)
        return QVariant();

    return item->data(role);
}

Qt::ItemFlags AppsModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QModelIndex AppsModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    AppNode *parentItem;
    if (!parent.isValid())
        parentItem = m_searchRoot ? m_searchRoot : m_root;
    else
        parentItem = static_cast<AppNode *>(parent.internalPointer());

    AppNode *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex AppsModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    AppNode *item = static_cast<AppNode *>(index.internalPointer());
    if (!item)
        return QModelIndex();

    AppNode *parentItem = item->parentItem();
    if (!parentItem || parentItem == m_searchRoot || parentItem == m_root)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int AppsModel::rowCount(const QModelIndex &parent) const
{
    AppNode *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = m_searchRoot ? m_searchRoot : m_root;
    else
        parentItem = static_cast<AppNode *>(parent.internalPointer());

    if (parentItem == nullptr)
        return 0;

    return parentItem->childCount();
}

int AppsModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}
}  // namespace Menu
}  // namespace Kiran