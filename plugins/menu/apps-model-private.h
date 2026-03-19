/**
 * Copyright (c) 2020 ~ 2025 KylinSec Co., Ltd.
 * kiran-shell is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     liuxinhao <liuxinhao@kylinsec.com.cn>
 */
#pragma once
#include <KConfigGroup>
#include <KDesktopFile>
#include <KServiceGroup>
#include <KSycoca>
#include <QElapsedTimer>
#include <QIcon>
#include <QThread>
#include "apps-model.h"
#include "icon-utils.h"
#include "ks-i.h"
#include "lib/common/logging-category.h"

#define DESKTOP_PROP_KIRAN_NO_DISPLAY "X-KIRAN-NoDisplay"
#define DESKTOP_PROP_NAME "Name"

namespace Kiran
{
namespace Menu
{
class AppNode
{
private:
    AppNode(const QMap<int, QVariant> &data, AppNode *parent = nullptr)
    {
        m_parentItem = parent;
        if (m_parentItem)
            m_parentItem->appendChild(this);
        m_itemData = data;
    }

public:
    static AppNode *createCategory(const QString &name, AppNode *parent = nullptr)
    {
        return new AppNode({{AppsModel::NameRole, name},
                            {AppsModel::TypeRole, AppsModel::ItemType::Category},
                            {AppsModel::IconRole, QIcon::fromTheme("ksvg-ks-menu-group-symbolic")}},
                           parent);
    }

    static AppNode *createNode(const QString &name, const QIcon &icon, const QString &id, const QString &tooltip, const QString &untranslatedName, bool fallbackIcon, AppNode *parent = nullptr)
    {
        return new AppNode({{AppsModel::NameRole, name},
                            {AppsModel::TypeRole, AppsModel::ItemType::Application},
                            {AppsModel::IconRole, icon},
                            {AppsModel::IdRole, id},
                            {AppsModel::ToolTipRole, tooltip},
                            {AppsModel::UntranslatedNameRole, untranslatedName},
                            {AppsModel::FallbackIconRole, fallbackIcon}},
                           parent);
    }

    static AppNode *createNode(const QMap<int, QVariant> &data, AppNode *parent = nullptr)
    {
        return new AppNode(data, parent);
    }

    ~AppNode()
    {
        qDeleteAll(m_childItems);
    }

    void appendChild(AppNode *child)
    {
        m_childItems.append(child);
        child->m_parentItem = this;
    }

    void prepend(AppNode *child)
    {
        m_childItems.prepend(child);
        child->m_parentItem = this;
    }

    void removeChild(AppNode *child)
    {
        m_childItems.removeAll(child);
    }

    AppNode *child(int row)
    {
        return m_childItems.value(row);
    }
    int childCount() const
    {
        return m_childItems.count();
    }

    int columnCount() const
    {
        return m_itemData.count();
    }

    QVariant data(int column) const
    {
        return m_itemData.value(column);
    }

    QMap<int, QVariant> data() const
    {
        return m_itemData;
    }

    int row() const
    {
        if (m_parentItem)
            return m_parentItem->m_childItems.indexOf(const_cast<AppNode *>(this));
        return 0;
    }

    AppNode *parentItem()
    {
        return m_parentItem;
    }

    static void cleanup(AppNode *item)
    {
        if (!item)
            return;

        // 递归清理所有子节点
        for (AppNode *child : item->m_childItems)
        {
            cleanup(child);
        }

        // 清空子节点列表并删除当前节点
        item->m_childItems.clear();
        delete item;
    }

private:
    AppNode *m_parentItem;
    QList<AppNode *> m_childItems;
    QMap<int, QVariant> m_itemData;
};

class AppsDataLoader : public QObject
{
    Q_OBJECT
public:
    AppsDataLoader(QObject *parent = nullptr)
        : QObject(parent)
    {
        KSycoca::self()->ensureCacheValid();
        qRegisterMetaType<QSet<QString>>("QSet<QString>");
        connect(KSycoca::self(), QOverload<>::of(&KSycoca::databaseChanged), this, &AppsDataLoader::loadData);
    }

signals:
    // 应用树已加载完成，信号槽中需要负责该应用树释放
    void dataLoaded(QSet<QString> appStorageIds, AppNode *newRootItem);
    void errorOccurred();

public slots:
    // 重新加载应用树
    void loadData()
    {
        QSet<QString> loadedAppStorageIds;
        auto serviceGroup = KServiceGroup::root();
        if (!serviceGroup || !serviceGroup->isValid())
        {
            emit errorOccurred();
            KLOG_ERROR(LCMenu) << "failed to load service group, root is invalid.";
            return;
        }

        // 形成应用树
        auto rootItem = AppNode::createCategory("Root");
        loadAppGroup(serviceGroup.data(), rootItem, loadedAppStorageIds);

        // 删除空白分类（没有子项的分类）
        removeEmptyCategories(rootItem);

        // 如果线程被中断，发出错误信号并清理资源
        if (QThread::currentThread()->isInterruptionRequested())
        {
            emit errorOccurred();
            AppNode::cleanup(rootItem);
            KLOG_ERROR(LCMenu) << "AppsDataLoader thread was interrupted, cleaning up resources.";
            return;
        }
        KLOG_DEBUG(LCMenu) << "AppsDataLoader loaded" << loadedAppStorageIds.size() << "applications";
        emit dataLoaded(loadedAppStorageIds, rootItem);
    }

private:
    void loadAppGroup(KSycocaEntry *parentEntry, AppNode *parentItem, QSet<QString> &storageIds)
    {
        auto parentGroup = static_cast<KServiceGroup *>(parentEntry);
        if (parentGroup->entries().isEmpty() || parentGroup->noDisplay())
            return;

        KServiceGroup::List entryList = parentGroup->entries();
        for (KServiceGroup::List::ConstIterator it = entryList.begin();
             it != entryList.end(); it++)
        {
            if (QThread::currentThread()->isInterruptionRequested())
            {
                return;
            }

            KSycocaEntry *entry = it->data();
            if (entry->isType(KST_KService))  // 应用
            {
                KService *service = static_cast<KService *>(entry);

                if (service->noDisplay())
                {
                    continue;
                }

                if (storageIds.contains(service->storageId()))
                {
                    continue;
                }

                KDesktopFile desktopFile(service->entryPath());
                const auto desktopGroupInfo = desktopFile.desktopGroup();
                auto kiranNoDisplay = service->property(DESKTOP_PROP_KIRAN_NO_DISPLAY, QVariant::Bool);
                auto untranslatedName = desktopGroupInfo.readEntryUntranslated(DESKTOP_PROP_NAME);

                if (kiranNoDisplay.isValid() && kiranNoDisplay.toBool())
                    continue;

                auto icon = loadAppIcon(service->icon());
                // 记录是否命中默认回退图标，供 AppsModel 后续决定是否触发重试刷新。
                bool fallbackIcon = false;
                if (icon.isNull())
                {
                    fallbackIcon = true;
                    icon = QIcon::fromTheme("application-x-executable");
                }

                storageIds << service->storageId();
                QString tooltip = service->name();
                // 当comment与name相同时，不显示comment
                if (!service->comment().isEmpty() && (service->name() != service->comment()))
                {
                    tooltip += "\n" + service->comment();
                }
                AppNode::createNode(service->name(), icon, service->storageId(), tooltip, untranslatedName, fallbackIcon, parentItem);
            }
            else if (entry->isType(KST_KServiceGroup))  // 分类
            {
                KServiceGroup *childGroup = static_cast<KServiceGroup *>(entry);
                AppNode *childGroupItem = AppNode::createCategory(childGroup->caption(), parentItem);
                loadAppGroup(childGroup, childGroupItem, storageIds);
            }
        }
    }

    void removeEmptyCategories(AppNode *item)
    {
        if (!item)
            return;

        for (int i = item->childCount() - 1; i >= 0; --i)
        {
            AppNode *child = item->child(i);
            auto type = child->data(AppsModel::TypeRole).toInt();

            if (type == AppsModel::ItemType::Application)
            {
                continue;
            }

            if (child->childCount() == 0)
            {
                item->removeChild(child);
                delete child;
                continue;
            }

            removeEmptyCategories(child);
        }
    }
};
}  // namespace Menu
}  // namespace Kiran