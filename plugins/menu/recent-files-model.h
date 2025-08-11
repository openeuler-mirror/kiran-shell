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
#include <QAbstractItemModel>
#include <QObject>
#include <QTimer>
#include <QVector>

namespace KActivities
{
namespace Stats
{
class ResultWatcher;
}
}  // namespace KActivities

class QThread;

namespace Kiran
{
namespace Menu
{
class RecentFilesLoader : public QObject
{
    Q_OBJECT
public:
    explicit RecentFilesLoader(QObject *parent = nullptr);

signals:
    // 数据加载完成，通知接收
    void dataLoaded(QVector<QMap<int, QVariant>> recentFilesData);
    void errorOccurred();

public slots:
    // 修改搜索关键字,更改关键字后，重新加载数据
    void changeKeyword(const QString &keyword);
    // 加载数据
    void loadData();

private:
    QString m_keyword;
    QTimer m_reloadTimer;
    KActivities::Stats::ResultWatcher *m_actStatsWatcher;
};

class RecentFilesModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum Roles
    {
        FileNameRole = Qt::DisplayRole,
        FileIconRole = Qt::DecorationRole,
        FilePathRole = Qt::UserRole + 1,
    };
    explicit RecentFilesModel(QObject *parent = nullptr);
    ~RecentFilesModel() override;

    void setFilterText(const QString &text);
    QString getFilePath(const QModelIndex &index = QModelIndex());

    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

signals:
    void changeKeyword(const QString &keyword);

private:
    void init();

private slots:
    void onDataLoaded(QVector<QMap<int, QVariant>> recentFilesData);
    void onErrorOccurred();

private:
    QThread *m_recentFilesLoadThread = nullptr;
    RecentFilesLoader *m_loader = nullptr;
    QVector<QMap<int, QVariant>> m_recentFilesData;
};

}  // namespace Menu
}  // namespace Kiran
