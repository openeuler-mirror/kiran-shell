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

#include <qt5-log-i.h>
#include <QGSettings>
#include <QScopedPointer>

#include "ks-definition.h"
#include "ks-i.h"
#include "layout.h"
#include "lib/common/logging-category.h"
#include "profile-applet.h"
#include "profile-panel.h"
#include "profile.h"
namespace Kiran
{
Profile* Profile::m_instance = nullptr;
void Profile::globalInit()
{
    m_instance = new Profile();
    m_instance->init();
}

void Profile::globalDeinit()
{
    delete m_instance;
    m_instance = nullptr;
}

Profile::Profile()
{
    m_settings = new QGSettings(SHELL_SCHEMA_ID, "", this);
    connect(m_settings, &QGSettings::changed, this, &Profile::updateSettings, Qt::QueuedConnection);
}

void Profile::init()
{
    // 事件循环尚未启动,手动同步建立对象表。
    // 此时无消费者连接信号, 无需 blockSignals。
    handleDefaultLayoutChanged();
    handlePanelUIDsChanged();

    if (m_panelUIDs.empty())
    {
        // 首次启动
        // panel-uids 为空, 从默认布局文件写入 per-panel/per-applet 配置与 UID 列表。
        // setter 仅写 GSettings; changed 因事件循环未启动无法异步到达, 手动调 handle* 同步生效, 幂等。
        loadFromLayout();
        handlePanelUIDsChanged();
    }

    // 先等待loadFromLayout读取默认布局写入gsettings, 再创建ProfileApplet对象, 避免读到空配置。
    handleAppletUIDsChanged();
}

void Profile::loadFromLayout()
{
    QScopedPointer<Layout> layout(new Layout(getDefaultLayout()));

    KLOG_DEBUG(LCShell) << "Load from layout " << getDefaultLayout();

    // per-panel/per-applet 属性必须先于 UID 列表写入 dconf, 否则 UID 列表
    // 一旦触发 ProfilePanel/ProfileApplet 创建, initSettings() 读到的仍是默认空值。
    QStringList panelUIDs;
    QStringList appletUIDs;

    auto layoutPanels = layout->getPanels();
    for (auto* layoutPanel : layoutPanels)
    {
        const QString& uid = layoutPanel->getUID();
        const QByteArray path = QString("%1/%2/").arg(PANEL_SCHEMA_PATH).arg(uid).toUtf8();
        QGSettings panelSettings(PANEL_SCHEMA_ID, path);
        panelSettings.set(PANEL_SCHEMA_KEY_SIZE, QVariant::fromValue(layoutPanel->getSize()));
        panelSettings.set(PANEL_SCHEMA_KEY_ORIENTATION, QVariant::fromValue(layoutPanel->getOrientation()));
        panelSettings.set(PANEL_SCHEMA_KEY_MONITOR, QVariant::fromValue(layoutPanel->getMonitor()));
        panelUIDs.append(uid);
    }

    auto layoutApplets = layout->getApplets();
    for (auto* layoutApplet : layoutApplets)
    {
        const QString& uid = layoutApplet->getUID();
        const QByteArray path = QString("%1/%2/").arg(APPLET_SCHEMA_PATH).arg(uid).toUtf8();
        QGSettings appletSettings(APPLET_SCHEMA_ID, path);
        appletSettings.set(APPLET_SCHEMA_KEY_ID, QVariant::fromValue(layoutApplet->getID()));
        appletSettings.set(APPLET_SCHEMA_KEY_PANEL, QVariant::fromValue(layoutApplet->getPanel()));
        appletSettings.set(APPLET_SCHEMA_KEY_POSITION, QVariant::fromValue(layoutApplet->getPosition()));
        appletSettings.set(APPLET_SCHEMA_KEY_PRS, QVariant::fromValue(layoutApplet->getPanelRightStick()));
        appletUIDs.append(uid);
    }

    setPanelUIDs(panelUIDs);
    setAppletUIDs(appletUIDs);
}

void Profile::setDefaultLayout(const QString& value)
{
    if (value == m_settings->get(SHELL_SCHEMA_KEY_DEFAULT_LAYOUT).toString())
        return;
    m_settings->set(SHELL_SCHEMA_KEY_DEFAULT_LAYOUT, QVariant::fromValue(value));
}

void Profile::handleDefaultLayoutChanged()
{
    QString newValue = m_settings->get(SHELL_SCHEMA_KEY_DEFAULT_LAYOUT).toString();
    if (newValue == m_defaultLayout)
        return;
    m_defaultLayout = newValue;
    Q_EMIT defaultLayoutChanged(newValue);
}

void Profile::setPanelUIDs(const QStringList& value)
{
    if (value == m_settings->get(SHELL_SCHEMA_KEY_PANEL_UIDS).toStringList())
        return;
    m_settings->set(SHELL_SCHEMA_KEY_PANEL_UIDS, QVariant::fromValue(value));
}

void Profile::setAppletUIDs(const QStringList& value)
{
    if (value == m_settings->get(SHELL_SCHEMA_KEY_APPLET_UIDS).toStringList())
        return;
    m_settings->set(SHELL_SCHEMA_KEY_APPLET_UIDS, QVariant::fromValue(value));
}

void Profile::handlePanelUIDsChanged()
{
    QStringList newValue = m_settings->get(SHELL_SCHEMA_KEY_PANEL_UIDS).toStringList();
    if (newValue == m_panelUIDs)
        return;
    m_panelUIDs = newValue;
    syncPanelObjects();
    Q_EMIT panelUIDsChanged(newValue);
}

void Profile::handleAppletUIDsChanged()
{
    QStringList newValue = m_settings->get(SHELL_SCHEMA_KEY_APPLET_UIDS).toStringList();
    if (newValue == m_appletUIDs)
        return;
    m_appletUIDs = newValue;
    syncAppletObjects();
    Q_EMIT appletUIDsChanged(newValue);
}

void Profile::syncPanelObjects()
{
    for (const QString& uid : m_panelUIDs)
    {
        if (!m_panels.contains(uid))
        {
            m_panels.insert(uid, QSharedPointer<ProfilePanel>::create(uid));
        }
    }
    for (const QString& uid : m_panels.keys())
    {
        if (!m_panelUIDs.contains(uid))
        {
            m_panels.remove(uid);
        }
    }
}

void Profile::syncAppletObjects()
{
    for (const QString& uid : m_appletUIDs)
    {
        if (!m_applets.contains(uid))
        {
            m_applets.insert(uid, QSharedPointer<ProfileApplet>::create(uid));
        }
    }
    for (const QString& uid : m_applets.keys())
    {
        if (!m_appletUIDs.contains(uid))
        {
            m_applets.remove(uid);
        }
    }
}

QList<ProfilePanelPtr> Profile::getPanels() const
{
    return m_panels.values();
}

QList<ProfileAppletPtr> Profile::getApplets() const
{
    return m_applets.values();
}

QList<ProfileAppletPtr> Profile::getAppletsOnPanel(const QString& panelUID) const
{
    QList<ProfileAppletPtr> applets;
    QList<ProfileAppletPtr> applets_right;

    for (const auto& applet : m_applets)
    {
        if (applet->getPanel() == panelUID)
        {
            if (applet->getPanelRightStick())
            {
                applets_right.push_back(applet);
            }
            else
            {
                applets.push_back(applet);
            }
        }
    }

    std::sort(applets.begin(), applets.end(), [](const ProfileAppletPtr& a, const ProfileAppletPtr& b)
              {
                  return a->getPosition() < b->getPosition();
              });

    // 右贴靠 applet 的 position 从右边缘起算, 数值大的离右边缘更近, 排序方向与左侧相反。
    std::sort(applets_right.begin(), applets_right.end(), [](const ProfileAppletPtr& a, const ProfileAppletPtr& b)
              {
                  return a->getPosition() > b->getPosition();
              });
    applets.append(applets_right);

    return applets;
}

void Profile::updateSettings(const QString& key)
{
    switch (shash(key.toUtf8().data()))
    {
    case CONNECT(SHELL_SCHEMA_KEY_DEFAULT_LAYOUT, _hash):
        handleDefaultLayoutChanged();
        break;
    case CONNECT(SHELL_SCHEMA_KEY_PANEL_UIDS, _hash):
        handlePanelUIDsChanged();
        break;
    case CONNECT(SHELL_SCHEMA_KEY_APPLET_UIDS, _hash):
        handleAppletUIDsChanged();
        break;
        GSETTINGS_CASE_DEFAULT
    }
}
}  // namespace Kiran
