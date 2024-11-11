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

Shelld::Shelld()
    : m_dirWatch(new KDirWatch(this)), m_timer(new QTimer(this))
{
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, []()
            {
                KSycoca::self()->ensureCacheValid();
                QProcess::startDetached("kbuildsycoca", {});
            });

    QObject::connect(m_dirWatch, &KDirWatch::dirty, this, &Shelld::update);
    QObject::connect(m_dirWatch, &KDirWatch::created, this, &Shelld::update);
    QObject::connect(m_dirWatch, &KDirWatch::deleted, this, &Shelld::update);
    QStringList dataDirs = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    for (auto &dir : dataDirs)
    {
        if (!m_dirWatch->contains(dir))
        {
            m_dirWatch->addDir(dir, KDirWatch::WatchDirOnly);
        }
    }
}

Shelld::~Shelld()
{
}

void Shelld::update(const QString &path)
{
    m_timer->start(2000);
}
