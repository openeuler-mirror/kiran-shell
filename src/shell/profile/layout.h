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
#include <QVariant>

class QSettings;

namespace Kiran
{
#define LAYOUT_PROPERTY_INT_DECLARATION(name, humpName) \
Q_SIGNALS:                                              \
    void name##Changed(int value);                      \
                                                        \
public:                                                 \
    int get##humpName();                                \
    void set##humpName(int value);

#define LAYOUT_PROPERTY_BOOLEAN_DECLARATION(name, humpName) \
Q_SIGNALS:                                                  \
    void name##Changed(bool value);                         \
                                                            \
public:                                                     \
    bool get##humpName();                                   \
    void set##humpName(bool value);

#define LAYOUT_PROPERTY_STRING_DECLARATION(name, humpName) \
Q_SIGNALS:                                                 \
    void name##Changed(const QString &value);              \
                                                           \
public:                                                    \
    QString get##humpName();                               \
    void set##humpName(const QString &value);

class Group : public QObject
{
    Q_OBJECT

public:
    Group(const QString &groupName, QSettings *settings, QObject *parent);

protected:
    void setValue(const QString &key, const QVariant &value);
    QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;

private:
    QString m_groupName;
    QSettings *m_settings;
};

class LayoutPanel : public Group
{
    Q_OBJECT

    LAYOUT_PROPERTY_INT_DECLARATION(size, Size);
    LAYOUT_PROPERTY_STRING_DECLARATION(orientation, Orientation);
    LAYOUT_PROPERTY_INT_DECLARATION(monitor, Monitor);

public:
    LayoutPanel(const QString &panelUID, QSettings *settings, QObject *parent);

    QString getUID()
    {
        return m_panelUID;
    }

private:
    QString m_panelUID;
};

class LayoutApplet : public Group
{
    Q_OBJECT

    LAYOUT_PROPERTY_STRING_DECLARATION(type, Type);
    LAYOUT_PROPERTY_STRING_DECLARATION(id, ID);
    LAYOUT_PROPERTY_STRING_DECLARATION(panel, Panel);
    LAYOUT_PROPERTY_INT_DECLARATION(position, Position);
    LAYOUT_PROPERTY_BOOLEAN_DECLARATION(panelRightStick, PanelRightStick);

public:
    LayoutApplet(const QString &appletUID, QSettings *settings, QObject *parent);

    QString getUID()
    {
        return m_appletUID;
    }

private:
    QString m_appletUID;
};

class Layout : public QObject
{
    Q_OBJECT
public:
    Layout(const QString &layoutName);

    QList<LayoutPanel *> getPanels();
    QList<LayoutApplet *> getApplets();
    QList<LayoutApplet *> getAppletsOnPanel(const QString &panelUID);

private:
    void load();

private:
    QSettings *m_settings;
    // 布局文件路径
    QString m_layoutFilePath;
    // panels
    QMap<QString, LayoutPanel *> m_panels;
    QMap<QString, LayoutApplet *> m_applets;
};

}  // namespace Kiran
