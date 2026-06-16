/**
 * Copyright (c) 2026 KylinSec Co., Ltd.
 * kiran-shell is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     Xinhao Liu <liuxinhao@kylinsec.com.cn>
 */

#pragma once

#include <QByteArray>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QString>

class DesktopFileCache : public QObject
{
    Q_OBJECT
public:
    static DesktopFileCache &instance();
    QByteArray findByAppId(const QString &appId);
    QByteArray findByPid(int pid);
    QByteArray findByDesktopEntryName(const QString &entryName);
    QByteArray findByExec(const QString &exec);

private:
    DesktopFileCache(QObject *parent = nullptr);
    void initServiceCache();
    void reloadServiceCache();
    void reloadServiceCacheUnlocked();
    QByteArray queryFromCache(const QString &info);

    QMap<QString, QByteArray> m_desktopEntryNameMap;
    QMap<QString, QByteArray> m_serviceNameMap;
    QMap<QString, QByteArray> m_execMap;
    QMap<QString, QByteArray> m_execSimpleMap;
    QMap<QString, QByteArray> m_startupWMClassMap;
    bool m_serviceCacheInitialized = false;
    QMutex m_cacheMutex;
};
