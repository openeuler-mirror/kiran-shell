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
#pragma once
#include <KSycocaEntry>
#include <QAbstractItemModel>
#include <QFuture>
#include <QFutureWatcher>
#include <QString>
#include <QThread>
#include <QVariant>
#include <QVector>

class QThread;
namespace Kiran
{
namespace Menu
{
class AppNode;
class AppsDataLoader;
class AppsModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum Roles
    {
        NameRole = Qt::DisplayRole,
        ToolTipRole = Qt::ToolTipRole,
        IconRole = Qt::DecorationRole,
        TypeRole = Qt::UserRole + 1,
        IdRole,
        UntranslatedNameRole,
        FallbackIconRole,
    };
    enum ItemType
    {
        Application,
        Category
    };
    explicit AppsModel(QObject *parent = nullptr);
    ~AppsModel() override;

    void setFilterText(const QString &text);

    QString getID(const QModelIndex &index = QModelIndex());
    bool isCategory(const QModelIndex &index = QModelIndex());
    QIcon getIcon(const QModelIndex &index = QModelIndex());

    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

private:
    void filter(AppNode *searchResult, const QString &searchKey, AppNode *item, QSet<QString> &storageIds);
    // 传入新应用集合以及原始应用节点和新应用根节点，构建新应用子树
    void buildNewAppsSubTree(QSet<QString> &newAppsSet, AppNode *parent, AppNode *newAppsNode);
    // 用于处理图标不显示的情况
    void collectFallbackIconApps(AppNode *parent, const QSet<QString> &targetAppIds, QSet<QString> &fallbackAppIds) const;
    void updateIconRetryState(AppNode *appsRoot, const QSet<QString> &newAppIds);
    void scheduleIconRetry();

private slots:
    void onDataLoaded(QSet<QString> appIds, AppNode *appsRoot);
    void onErrorOccurred();

private:
    // 数据加载线程
    QThread *m_dataLoadThread = nullptr;
    AppsDataLoader *m_dataLoader = nullptr;
    // 原始数据
    AppNode *m_root = nullptr;
    QSet<QString> m_appIds;
    // 搜索结果
    AppNode *m_searchRoot = nullptr;
    QSet<QString> m_pendingIconRetryAppIds;
    int m_remainingIconRetryCount = 0;
    bool m_iconRetryScheduled = false;
};
}  // namespace Menu
}  // namespace Kiran