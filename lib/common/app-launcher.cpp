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

#include <KActivities/KActivities/ResourceInstance>
#include <KIO/ApplicationLauncherJob>

#include "app-launcher.h"

namespace Kiran
{
namespace Common
{
static void appStart(KIO::ApplicationLauncherJob *job, QString storageId, QList<QUrl> urls)
{
    if (!urls.isEmpty())
    {
        job->setUrls(urls);
    }

    job->start();

    // 通知kactivitymanagerd
    KActivities::ResourceInstance::notifyAccessed(
        QUrl(QStringLiteral("applications:") + storageId));
}

void appLauncher(const KService::Ptr &service, QList<QUrl> urls)
{
    auto *job = new KIO::ApplicationLauncherJob(service);
    appStart(job, service->storageId(), urls);
}

void appLauncher(const KServiceAction &serviceAction, QString storageId, QList<QUrl> urls)
{
    auto *job = new KIO::ApplicationLauncherJob(serviceAction);
    appStart(job, storageId, urls);
}

}  // namespace Common
}  // namespace Kiran
