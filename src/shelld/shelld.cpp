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
 * Author:     yangfeng <yangfeng@kylinsec.com.cn>
 */

#include <KDirWatch>
#include <KSycoca>
#include <QProcess>
#include <QStandardPaths>
#include <QTimer>

#include "shelld.h"
#include "status-notifier-watcher.h"

static constexpr unsigned int SYCOCA_BUILD_TIME = 2000;

namespace Kiran
{
Shelld::Shelld()
    : m_dirWatch(new KDirWatch(this)), m_timer(new QTimer(this))
{
    // 监控系统安装的应用程序变化
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, []()
            {
                KSycoca::self()->ensureCacheValid();
                QProcess::startDetached("kbuildsycoca5", {});
            });

    QObject::connect(m_dirWatch, &KDirWatch::dirty, this, &Shelld::update);
    QObject::connect(m_dirWatch, &KDirWatch::created, this, &Shelld::update);
    QObject::connect(m_dirWatch, &KDirWatch::deleted, this, &Shelld::update);
    QStringList dataDirs = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);

    for (auto &dir : dataDirs)
    {
        if (!m_dirWatch->contains(dir))
        {
            m_dirWatch->addDir(dir, KDirWatch::WatchDirOnly);
        }
    }

    // 托盘服务
    new StatusNotifierWatcher(this);
}

void Shelld::update(const QString &path)
{
    m_timer->start(SYCOCA_BUILD_TIME);
}
}  // namespace Kiran
