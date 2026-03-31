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
#include "recent-files-model.h"
#include <KActivities/Stats/Cleaning>
#include <KActivities/Stats/ResultModel>
#include <KActivities/Stats/ResultSet>
#include <KActivities/Stats/ResultWatcher>
#include <KService/KService>
#include <QFile>
#include <QFileInfo>
#include <QIcon>
#include <QMimeDatabase>
#include <QThread>
#include <QTimer>
#include "lib/common/icon-utils.h"
#include "lib/common/logging-category.h"

namespace KAStats = KActivities::Stats;
using namespace KAStats;
using namespace KAStats::Terms;

static const Query RECENT_FILE_QUERY = UsedResources | RecentlyUsedFirst | Agent::any() | Type::files() | Activity::any() | Url::file();

namespace Kiran
{
namespace Menu
{
RecentFilesLoader::RecentFilesLoader(QObject *parent)
    : QObject(parent),
      m_actStatsWatcher(new KActivities::Stats::ResultWatcher(RECENT_FILE_QUERY, this))
{
    qRegisterMetaType<QVector<QMap<int, QVariant>>>("QVector<QMap<int, QVariant>>");
    m_reloadTimer.setInterval(5000);
    m_reloadTimer.setSingleShot(true);

    connect(m_actStatsWatcher, &KActivities::Stats::ResultWatcher::resultScoreUpdated, &m_reloadTimer, QOverload<>::of(&QTimer::start));
    connect(m_actStatsWatcher, &KActivities::Stats::ResultWatcher::resultRemoved, &m_reloadTimer, QOverload<>::of(&QTimer::start));
    connect(m_actStatsWatcher, &KActivities::Stats::ResultWatcher::resultLinked, &m_reloadTimer, QOverload<>::of(&QTimer::start));
    connect(m_actStatsWatcher, &KActivities::Stats::ResultWatcher::resultUnlinked, &m_reloadTimer, QOverload<>::of(&QTimer::start));
    connect(&m_reloadTimer, &QTimer::timeout, this, &RecentFilesLoader::loadData);
}

void RecentFilesLoader::changeKeyword(const QString &keyword)
{
    if (keyword != m_keyword)
    {
        m_keyword = keyword;
        loadData();
    }
}

void RecentFilesLoader::loadData()
{
    const auto query = RECENT_FILE_QUERY | Url::contains(m_keyword);
    QVector<QMap<int, QVariant>> recentFilesData;
    for (const ResultSet::Result &result : ResultSet(query))
    {
        QString filePath = QUrl(result.resource()).path();
        QFileInfo fileInfo(filePath);
        if (!QFile::exists(filePath) || fileInfo.isDir())
        {
            continue;
        }

        // 查询文件MimeType，根据MimeType尝试加载Freedesktop图标名
        auto mimeType = QMimeDatabase().mimeTypeForFile(fileInfo);
        auto icon = Kiran::loadIcon(mimeType.iconName());
        if (icon.isNull())
        {
            // fallback 通用类型图标名
            icon = Kiran::loadIcon(mimeType.genericIconName());
        }

        QMap<int, QVariant> fileData;
        fileData.insert(RecentFilesModel::FileNameRole, result.title());
        fileData.insert(RecentFilesModel::FileIconRole, icon);
        fileData.insert(RecentFilesModel::FilePathRole, filePath);
        recentFilesData.append(fileData);
    }
    KLOG_DEBUG(LCMenu) << "recent files loaded:" << recentFilesData.size() << "items";
    emit dataLoaded(recentFilesData);
}

RecentFilesModel::RecentFilesModel(QObject *parent)
{
    init();
}

RecentFilesModel::~RecentFilesModel()
{
    if (m_recentFilesLoadThread)
    {
        m_recentFilesLoadThread->quit();
        if (!m_recentFilesLoadThread->wait(1000))
        {
            m_recentFilesLoadThread->terminate();
            m_recentFilesLoadThread->wait();
        }
        delete m_loader;
        m_loader = nullptr;
    }
}

void RecentFilesModel::setFilterText(const QString &text)
{
    emit changeKeyword(text);
}

void RecentFilesModel::init()
{
    m_recentFilesLoadThread = new QThread(this);
    m_loader = new RecentFilesLoader();
    m_loader->moveToThread(m_recentFilesLoadThread);

    connect(m_recentFilesLoadThread, &QThread::started, m_loader, &RecentFilesLoader::loadData);
    connect(this, &RecentFilesModel::changeKeyword, m_loader, &RecentFilesLoader::changeKeyword);
    connect(m_loader, &RecentFilesLoader::dataLoaded, this, &RecentFilesModel::onDataLoaded);
    connect(m_loader, &RecentFilesLoader::errorOccurred, this, &RecentFilesModel::onErrorOccurred);

    m_recentFilesLoadThread->start();
}

void RecentFilesModel::onDataLoaded(QVector<QMap<int, QVariant>> recentFilesData)
{
    beginResetModel();
    m_recentFilesData = recentFilesData;
    endResetModel();
    emit dataChanged(index(0, 0), index(rowCount() - 1, 0));
}

void RecentFilesModel::onErrorOccurred()
{
}

QString RecentFilesModel::getFilePath(const QModelIndex &index)
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_recentFilesData.size())
    {
        return QString();
    }
    return m_recentFilesData.at(index.row()).value(FilePathRole).toString();
}

QVariant RecentFilesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0)
    {
        return QVariant();
    }

    const auto &recentFiles = m_recentFilesData;
    if (index.row() >= recentFiles.size())
    {
        return QVariant();
    }

    const auto &fileData = recentFiles.at(index.row());
    switch (role)
    {
    case FileNameRole:
        return fileData.value(FileNameRole);
    case FileIconRole:
        return fileData.value(FileIconRole);
    case FilePathRole:
        return fileData.value(FilePathRole);
    case Qt::ToolTipRole:
        return fileData.value(FileNameRole).toString() + "\n" + fileData.value(FilePathRole).toString();
    default:
        return QVariant();
    }
}

Qt::ItemFlags RecentFilesModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QModelIndex RecentFilesModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex RecentFilesModel::parent(const QModelIndex &index) const
{
    return QModelIndex();
}

int RecentFilesModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;
    if (parent != QModelIndex())
        return 0;
    return m_recentFilesData.size();
}

int RecentFilesModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

void RecentFilesModel::removeFile(QString filePath)
{
    KAStats::forgetResource(Activity::any(), Agent::any(), filePath);
    m_loader->loadData();
}

void RecentFilesModel::removeAll()
{
    KAStats::forgetResources(RECENT_FILE_QUERY);
    m_loader->loadData();
}

}  // namespace Menu
}  // namespace Kiran
