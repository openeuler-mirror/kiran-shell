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
#include <QDebug>
#include <QProcess>

#include "app-launcher.h"
#include "ks-i.h"

namespace Kiran
{
namespace Common
{
static void appStart(const KService::Ptr &service, QList<QUrl> urls)
{
    auto *job = new KIO::ApplicationLauncherJob(service);
    if (!urls.isEmpty())
    {
        job->setUrls(urls);
    }

    job->start();
}

static bool appStart(QString exec, QString entryPath, QList<QUrl> urls, bool isTerminal = false)
{
    QProcess p;
    QStringList args;

    if (isTerminal)
    {
        p.setProgram("mate-terminal");
        args.append("-e");
        args.append(exec);
    }
    else
    {
        p.setProgram(exec);
    }

    for (auto url : urls)
    {
        args.append(url.toString());
    }

    p.setArguments(args);

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert(APP_LAUNCHED_PREFIX, entryPath);
    p.setProcessEnvironment(env);

    return p.startDetached();
}

void appLauncher(const KService::Ptr &service, QList<QUrl> urls)
{
    QString storageId = service->storageId();

    if (!appStart(service->exec(), service->entryPath(), urls, service->terminal()))
    {
        service->setExec(APP_LAUNCHED_PREFIX + "=" + service->entryPath() + " " + service->exec());
        appStart(service, urls);
    }

    // 通知kactivitymanagerd
    KActivities::ResourceInstance::notifyAccessed(
        QUrl(QStringLiteral("applications:") + storageId));
}

void appLauncher(const KServiceAction &serviceAction, QString storageId, QList<QUrl> urls)
{
    auto service = serviceAction.service();

    if (!appStart(serviceAction.exec(), service->entryPath(), urls, service->terminal()))
    {
        service->setExec(APP_LAUNCHED_PREFIX + "=" + service->entryPath() + " " + serviceAction.exec());
        appStart(service, urls);
    }

    // 通知kactivitymanagerd
    KActivities::ResourceInstance::notifyAccessed(
        QUrl(QStringLiteral("applications:") + storageId));
}

}  // namespace Common
}  // namespace Kiran
