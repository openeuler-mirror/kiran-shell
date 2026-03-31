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
#include <KX11Extras>
#include "lib/common/icon-utils.h"
#include "lib/common/logging-category.h"
#include "lib/common/window-info-helper.h"

namespace Kiran
{
bool getAppInfo(WId wid, AppInfo &info)
{
    QUrl url = WindowInfoHelper::getUrlByWId(wid);
    QByteArray wmClass = WindowInfoHelper::getWmClassByWId(wid);
    if (url.isEmpty() && wmClass.isEmpty())
    {
        KLOG_WARNING() << "can't find url and wmclass by wid:" << wid;
        return false;
    }

    info = AppInfo(url, wmClass);
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

    const QString iconName = WindowInfoHelper::getAppIconByWId(wid);
    if (!iconName.isEmpty())
    {
        const QIcon icon = loadIcon(iconName);
        if (!icon.isNull())
        {
            return icon.pixmap(size);
        }
    }

    return KX11Extras::icon(wid, size.width(), size.height(), true);
}
}  // namespace Kiran
