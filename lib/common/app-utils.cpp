/**
 * Copyright (c) 2026 ~ 2027 KylinSec Co., Ltd.
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

#include "lib/common/app-utils.h"
#include <KIOCore/KFileItem>
#include "lib/common/desktop-file-cache.h"
#include "lib/common/icon-utils.h"
#include "lib/common/logging-category.h"
#include "lib/common/window-manager.h"

namespace Kiran
{
bool getAppInfo(WId wid, AppInfo &info)
{
    QByteArray desktopFile;

    // 1. 窗口属性直接提供 desktop file
    desktopFile = WindowManagerInstance.getWindowDesktopFileName(wid);

    // 2. 通过 appId 查 KService 缓存
    if (desktopFile.isEmpty())
    {
        QString appId = WindowManagerInstance.getWindowAppId(wid);
        desktopFile = DesktopFileCache::instance().findByAppId(appId);
    }

    // 3. 通过 PID 查 cmdline/environ
    if (desktopFile.isEmpty())
    {
        int pid = WindowManagerInstance.getWindowPid(wid);
        if (pid > 0)
        {
            desktopFile = DesktopFileCache::instance().findByPid(pid);
        }
    }

    QString appId = WindowManagerInstance.getWindowAppId(wid);
    QUrl url = QUrl::fromLocalFile(desktopFile);

    if (url.isEmpty() && appId.isEmpty())
    {
        KLOG_WARNING() << "can't find url and appId by wid:" << wid;
        return false;
    }

    info = AppInfo(url, appId);
    return true;
}

QPixmap getWindowAppIcon(WId wid, const QSize &size)
{
    AppInfo appInfo;
    if (getAppInfo(wid, appInfo) && !appInfo.m_url.isEmpty())
    {
        KFileItem fileItem(appInfo.m_url);
        if (!fileItem.isNull())
        {
            const QIcon icon = loadIcon(fileItem.iconName());
            if (!icon.isNull())
            {
                return icon.pixmap(size);
            }
        }
    }

    const QString iconName = WindowManagerInstance.getWindowIconName(wid);
    if (!iconName.isEmpty())
    {
        const QIcon icon = loadIcon(iconName);
        if (!icon.isNull())
        {
            return icon.pixmap(size);
        }
    }

    return WindowManagerInstance.getWindowIcon(wid, size);
}
}  // namespace Kiran
