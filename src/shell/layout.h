/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd. 
 * kiran-session-manager is licensed under Mulan PSL v2.
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
#include <QSettings>
#include <QSharedPointer>
#include <QVariant>

namespace Kiran
{
namespace Model
{
#define LAYOUT_PROPERTY_INT(name, humpName, key) \
Q_SIGNALS:                                       \
    void name##Changed(int value);               \
                                                 \
public:                                          \
    int get##humpName()                          \
    {                                            \
        return this->value(#key).toInt();        \
    }                                            \
    void set##humpName(int value)                \
    {                                            \
        if (value != this->get##humpName())      \
        {                                        \
            this->setValue(#key, value);         \
        }                                        \
    }

#define LAYOUT_PROPERTY_STRING(name, humpName, key) \
Q_SIGNALS:                                          \
    void name##Changed(const QString &value);       \
                                                    \
public:                                             \
    QString get##humpName()                         \
    {                                               \
        return this->value(#key).toString();        \
    }                                               \
    void set##humpName(const QString &value)        \
    {                                               \
        if (value != this->get##humpName())         \
        {                                           \
            this->setValue(#key, value);            \
        }                                           \
    }

class Group : public QObject
{
    Q_OBJECT

public:
    Group(const QString &groupName, QSettings *settings);

protected:
    void setValue(const QString &key, const QVariant &value);
    QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;

private:
    QString m_groupName;
    QSettings *m_settings;
};

class Panel : public Group
{
    Q_OBJECT

    LAYOUT_PROPERTY_INT(size, Size, size);
    LAYOUT_PROPERTY_STRING(orientation, Orientation, orientation);
    LAYOUT_PROPERTY_INT(monitor, Monitor, monitor);

public:
    Panel(const QString &panelUID, QSettings *settings);

    QString getUID()
    {
        return this->m_panelUID;
    }

private:
    QString m_panelUID;
};

class Applet : public Group
{
    Q_OBJECT

    LAYOUT_PROPERTY_STRING(type, Type, type);
    LAYOUT_PROPERTY_STRING(id, ID, id);
    LAYOUT_PROPERTY_STRING(panel, Panel, panel);
    LAYOUT_PROPERTY_INT(position, Position, position);

public:
    Applet(const QString &appletUID, QSettings *settings);

    QString getUID()
    {
        return this->m_appletUID;
    }

private:
    QString m_appletUID;
};

class Layout : public QObject
{
    Q_OBJECT
public:
    static Layout *getInstance();
    Layout();

    QList<QSharedPointer<Panel>> getPanels();
    // 创建的面板配置信息会直接落地到磁盘
    // QSharedPointer<Panel> newPanel();
    // QSharedPointer<Panel> getPanel(const QString &panelUID);
    // void removePanel(const QString &panelUID);

    // QList<QSharedPointer<Applet>> getApplets();
    QList<QSharedPointer<Applet>> getAppletsOnPanel(const QString &panelUID);
    // QSharedPointer<Applet> newApplet();
    // QSharedPointer<Applet> getApplet(const QString &appletUID);
    // void removeApplet(const QString &appletUID);

private:
    void loadLayout();
    void updateLayoutFile();
    void resetLayoutFile();

private:
    QSettings *m_settings;
    // 布局文件路径
    QString m_layoutFilePath;
    // panels
    QMap<QString, QSharedPointer<Panel>> m_panels;
    QMap<QString, QSharedPointer<Applet>> m_applets;
};
}  // namespace Model

}  // namespace Kiran
