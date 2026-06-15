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
 */

#pragma once

#include <QByteArray>
#include <QDebug>
#include <QPixmap>
#include <QSize>
#include <QString>
#include <QUrl>
#include <QWidgetList>

namespace Kiran
{

enum AppIdType
{
    APP_ID_TYPE_DESKTOP = 0,
    APP_ID_TYPE_WMCLASS
};

class AppInfo
{
public:
    QString m_id;
    AppIdType m_idType;

    QString m_appId;
    QUrl m_url;

    AppInfo() = default;

    AppInfo(QUrl url, QString appId)
        : m_appId(std::move(appId)), m_url(std::move(url))
    {
        if (!m_url.isEmpty() && m_url.isValid())
        {
            m_idType = APP_ID_TYPE_DESKTOP;
            m_id = m_url.toString();
        }
        else
        {
            m_idType = APP_ID_TYPE_WMCLASS;
            m_id = m_appId;
        }
    }

    AppInfo &operator=(const AppInfo &other) = default;

    AppInfo(const AppInfo &other)
        : m_id(other.m_id), m_idType(other.m_idType), m_appId(other.m_appId), m_url(other.m_url) {}

    friend QDebug operator<<(QDebug dbg, const AppInfo &other)
    {
        QDebugStateSaver saver(dbg);

        dbg.nospace() << "AppInfo(" << other.m_id << other.m_appId << "," << other.m_url.toString() << ")";
        return dbg.space();
    }

    // 重载==运算
    bool operator==(const AppInfo &other) const
    {
        return m_id == other.m_id;
    }

    bool operator!=(const AppInfo &other) const
    {
        return !(*this == other);
    }

    // 重载<运算，以便map使用
    bool operator<(const AppInfo &other) const
    {
        return m_id < other.m_id;
    }
};

bool getAppInfo(WId wid, AppInfo &info);
QPixmap getWindowAppIcon(WId wid, const QSize &size);

}  // namespace Kiran