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
