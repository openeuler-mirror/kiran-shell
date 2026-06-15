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

#include <qt5-log-i.h>
#include <KService/KService>
#include <KSycoca>
#include <QFileInfo>
#include <QMutexLocker>

#include "desktop-file-cache.h"
#include "ks-i.h"
#include "logging-category.h"
#include "utility.h"

DesktopFileCache &DesktopFileCache::instance()
{
    static DesktopFileCache cache;
    return cache;
}

DesktopFileCache::DesktopFileCache(QObject *parent)
    : QObject(parent)
{
}

void DesktopFileCache::initServiceCache()
{
    QMutexLocker locker(&m_cacheMutex);

    if (m_serviceCacheInitialized)
    {
        return;
    }

    KSycoca::self()->ensureCacheValid();

    static bool signalConnected = false;
    if (!signalConnected)
    {
        connect(KSycoca::self(), QOverload<>::of(&KSycoca::databaseChanged),
                [this]()
                {
                    KLOG_DEBUG(LCLib) << "KSycoca database changed, reloading service cache";
                    reloadServiceCache();
                });
        signalConnected = true;
    }

    reloadServiceCacheUnlocked();
}

void DesktopFileCache::reloadServiceCache()
{
    QMutexLocker locker(&m_cacheMutex);
    reloadServiceCacheUnlocked();
}

void DesktopFileCache::reloadServiceCacheUnlocked()
{
    m_desktopEntryNameMap.clear();
    m_serviceNameMap.clear();
    m_execMap.clear();
    m_execSimpleMap.clear();
    m_startupWMClassMap.clear();

    KLOG_INFO(LCLib) << "Loading KService cache...";
    const auto allKService = KService::allServices();

    for (const auto &service : allKService)
    {
        const QByteArray entryPath = service->entryPath().toLocal8Bit();
        if (entryPath.isEmpty())
        {
            continue;
        }

        const QString desktopEntryName = service->desktopEntryName();
        if (!desktopEntryName.isEmpty())
        {
            m_desktopEntryNameMap.insert(desktopEntryName, entryPath);
        }

        const QString serviceName = service->name();
        if (!serviceName.isEmpty())
        {
            m_serviceNameMap.insert(serviceName, entryPath);
        }

        const QString exec = service->exec();
        if (!exec.isEmpty())
        {
            if (!m_execMap.contains(exec))
            {
                m_execMap.insert(exec, entryPath);
            }

            const int spacePos = exec.indexOf(' ');
            const QString execSimple = (spacePos > 0) ? exec.left(spacePos) : exec;
            if (!execSimple.isEmpty())
            {
                if (!m_execSimpleMap.contains(execSimple))
                {
                    m_execSimpleMap.insert(execSimple, entryPath);
                }
            }
        }

        const QString startupWMClass = service->property(QStringLiteral("StartupWMClass")).toString();
        if (!startupWMClass.isEmpty())
        {
            m_startupWMClassMap.insert(startupWMClass, entryPath);
        }
    }

    m_serviceCacheInitialized = true;
    KLOG_INFO(LCLib) << "KService cache loaded. "
                     << "desktopEntryName:" << m_desktopEntryNameMap.size()
                     << "serviceName:" << m_serviceNameMap.size()
                     << "exec:" << m_execMap.size()
                     << "execSimple:" << m_execSimpleMap.size()
                     << "startupWMClass:" << m_startupWMClassMap.size();
}

QByteArray DesktopFileCache::queryFromCache(const QString &info)
{
    // 按优先级一级一级查找，不能在一个循环中做所有的查找，不然优先级低的可能会先命中
    // 为什么？：desktopEntryName只能是唯一的，其他字段不同软件可能会相同，我们按 name exec 的顺序排列优先级
    // 1.service->desktopEntryName()
    // 2.service->name()
    // 3.service->exec()
    // 4.StartupWMClass
    // StartupWMClass当KService没有匹配成功时再尝试，例如通过chrome打开wps文档，使用的二进制不在上述三种情况之中 #100994

    // 按优先级一级一级查询
    // 第一级：desktopEntryName
    auto it = m_desktopEntryNameMap.constFind(info);
    if (it != m_desktopEntryNameMap.constEnd())
    {
        return it.value();
    }

    // 第二级：name
    it = m_serviceNameMap.constFind(info);
    if (it != m_serviceNameMap.constEnd())
    {
        return it.value();
    }

    // 第三级：exec (完整匹配)
    it = m_execMap.constFind(info);
    if (it != m_execMap.constEnd())
    {
        return it.value();
    }

    // 第三级：exec_simple (精确匹配或前缀匹配)
    // 优化：使用 const_iterator 和提前计算字符串长度，避免重复比较
    const int infoLength = info.length();
    for (auto mapIt = m_execSimpleMap.constBegin(); mapIt != m_execSimpleMap.constEnd(); ++mapIt)
    {
        const QString &execSimple = mapIt.key();
        const int execSimpleLength = execSimple.length();

        // 精确匹配
        if (info == execSimple)
        {
            return mapIt.value();
        }

        // 前缀匹配：info 以 execSimple 开头，或 execSimple 以 info 开头
        if ((execSimpleLength > 0 && infoLength >= execSimpleLength && info.startsWith(execSimple)) ||
            (infoLength > 0 && execSimpleLength >= infoLength && execSimple.startsWith(info)))
        {
            return mapIt.value();
        }
    }

    // 第四级：StartupWMClass
    it = m_startupWMClassMap.constFind(info);
    if (it != m_startupWMClassMap.constEnd())
    {
        return it.value();
    }

    return QByteArray();
}

QByteArray DesktopFileCache::findByAppId(const QString &appId)
{
    if (!m_serviceCacheInitialized)
    {
        initServiceCache();
    }

    QByteArray result;
    {
        QMutexLocker locker(&m_cacheMutex);
        result = queryFromCache(appId);
    }

    return result;
}

QByteArray DesktopFileCache::findByDesktopEntryName(const QString &entryName)
{
    if (!m_serviceCacheInitialized)
    {
        initServiceCache();
    }

    QMutexLocker locker(&m_cacheMutex);
    auto it = m_desktopEntryNameMap.constFind(entryName);
    if (it != m_desktopEntryNameMap.constEnd())
    {
        return it.value();
    }
    return QByteArray();
}

QByteArray DesktopFileCache::findByExec(const QString &exec)
{
    if (exec.isEmpty())
    {
        return QByteArray();
    }

    if (!m_serviceCacheInitialized)
    {
        initServiceCache();
    }

    QByteArray result;
    {
        QMutexLocker locker(&m_cacheMutex);
        result = queryFromCache(exec);
    }

    return result;
}

QByteArray DesktopFileCache::findByPid(int pid)
{
    // 优先从 environ 中读取 KIRAN_SHELL_LAUNCHED_DESKTOP_FILE
    QByteArray environ = Utility::runCmd("cat", {QString("/proc/%1/environ").arg(pid)});
    QByteArrayList envsList = environ.split('\0');
    for (const auto &env : envsList)
    {
        if (env.startsWith(APP_LAUNCHED_PREFIX))
        {
            return env.mid(APP_LAUNCHED_PREFIX.length() + 1);
        }
    }

    // 回退：从 cmdline 提取 exec 名后通过 findByExec 查询
    QByteArray cmdline = Utility::runCmd("cat", {QString("/proc/%1/cmdline").arg(pid)});
    QByteArrayList parts = cmdline.split(' ');
    QString cmd = parts.first();
    if (cmd.isEmpty())
    {
        return QByteArray();
    }
    QString cmdSimple = QFileInfo(cmd).fileName();

    return findByExec(cmdSimple);
}
