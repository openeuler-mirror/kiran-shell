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
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#include "plugin.h"
#include <qt5-log-i.h>
#include <QDir>
#include <QGlobalStatic>
#include <QJsonArray>
#include <QJsonObject>
#include <QPluginLoader>
#include <QScopedPointer>
#include "ks-config.h"
#include "utils.h"

namespace Kiran
{
struct AppletMetaData
{
    QString appletID;
};

struct PluginMetaData
{
    QString pluginID;
    QMap<QString, AppletMetaData> appletsMetaData;
};

struct PluginInfo
{
    QSharedPointer<QPluginLoader> loader;
    PluginMetaData metaData;
};

Q_GLOBAL_STATIC(PluginPool, gs_pluginPool)
PluginPool *PluginPool::getInstance()
{
    return gs_pluginPool;
}

PluginPool::PluginPool()
{
    this->loadPluginsMeta();
}

IPlugin *PluginPool::findPluginForApplet(const QString &appletID)
{
    // 搜索meta信息，这里数据量一般比较少，所以直接遍历了
    for (auto iter = this->m_plugins.begin(); iter != this->m_plugins.end(); ++iter)
    {
        if ((*iter)->metaData.appletsMetaData.find(appletID) != (*iter)->metaData.appletsMetaData.end())
        {
            auto instance = (*iter)->loader->instance();
            //            qWarning() << (*iter)->loader->metaData();
            //            qWarning() << instance;

            IPlugin *ptr = qobject_cast<IPlugin *>(instance);

            return ptr;
        }
    }
    return nullptr;
}

void PluginPool::loadPluginsMeta()
{
    QDir dir(KS_INSTALL_PLUGINDIR);

    for (auto &entryInfo : dir.entryInfoList())
    {
        CONTINUE_IF_TRUE(entryInfo.isDir());

        if (!entryInfo.fileName().endsWith(".so"))
        {
            KLOG_WARNING() << "Ignore file " << entryInfo.absoluteFilePath() << ", because it doesn't end with so.";
            continue;
        }

        KLOG_DEBUG() << "Load metadata for plugin " << entryInfo.absoluteFilePath();

        auto pluginInfo = QSharedPointer<PluginInfo>(new PluginInfo());
        pluginInfo->loader = QSharedPointer<QPluginLoader>::create(entryInfo.absoluteFilePath());
        auto metaDataJson = pluginInfo->loader->metaData();
        metaDataJson = metaDataJson.value("MetaData").toObject();
        pluginInfo->metaData.pluginID = metaDataJson.value("plugin_id").toString();
        auto appletsJson = metaDataJson.value("applets").toArray();
        for (auto iter = appletsJson.begin(); iter != appletsJson.end(); ++iter)
        {
            AppletMetaData appletMetaData;
            appletMetaData.appletID = (*iter).toObject().value("applet_id").toString();
            pluginInfo->metaData.appletsMetaData.insert(appletMetaData.appletID, appletMetaData);
        }
        this->m_plugins.insert(entryInfo.absoluteFilePath(), pluginInfo);
    }
}

}  // namespace Kiran
