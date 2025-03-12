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
