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

#pragma once

#include <QMap>
#include <QSharedPointer>
#include "profile-applet.h"
#include "profile-panel.h"

class QGSettings;

namespace Kiran
{
using ProfilePanelPtr = QSharedPointer<ProfilePanel>;
using ProfileAppletPtr = QSharedPointer<ProfileApplet>;

class Profile : public QObject
{
    Q_OBJECT
public:
    static Profile* getInstance()
    {
        return m_instance;
    };

    static void globalInit();
    static void globalDeinit();

    QString getDefaultLayout() const
    {
        return m_defaultLayout;
    }
    QStringList getPanelUIDs() const
    {
        return m_panelUIDs;
    }
    QStringList getAppletUIDs() const
    {
        return m_appletUIDs;
    }
    QList<ProfilePanelPtr> getPanels() const;
    QList<ProfileAppletPtr> getApplets() const;
    QList<ProfileAppletPtr> getAppletsOnPanel(const QString& panelUID) const;

    void setDefaultLayout(const QString& value);
    void setPanelUIDs(const QStringList& value);
    void setAppletUIDs(const QStringList& value);

Q_SIGNALS:
    void defaultLayoutChanged(const QString& value);
    void panelUIDsChanged(const QStringList& value);
    void appletUIDsChanged(const QStringList& value);

private:
    Profile();

    void init();
    // 仅把默认布局翻译成 GSettings 键值对, 不创建/持有任何 ProfilePanel/ProfileApplet。
    // 对象创建完全交给 GSettings changed 响应路径, 与"外部 dconf 写入"同一路径。
    void loadFromLayout();

    void handleDefaultLayoutChanged();
    void handlePanelUIDsChanged();
    void handleAppletUIDsChanged();

    void syncPanelObjects();
    void syncAppletObjects();

private slots:
    void updateSettings(const QString& key);

private:
    static Profile* m_instance;
    QString m_defaultLayout;
    QStringList m_panelUIDs;
    QStringList m_appletUIDs;
    QGSettings* m_settings;
    QMap<QString, ProfilePanelPtr> m_panels;
    QMap<QString, ProfileAppletPtr> m_applets;
};
}  // namespace Kiran
