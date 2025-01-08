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
 * Author:     yanglan <yanglan@kylinos.com.cn>
 */

#pragma once

#include <kiran-color-block.h>
#include <ks-i.h>
#include <plugin-i.h>
#include <QPushButton>

class QDBusInterface;
class QTimer;
class StyledButton;
namespace Kiran
{
namespace Calendar
{
class Window;
class CalendarButton;

class Applet : public KiranColorBlock
{
    Q_OBJECT

public:
    Applet(IAppletImport *import);
    ~Applet();

private slots:
    void timeInfoChanged();
    // panel布局信息发生变化
    void updateLayout();

private:
    void initTimeDbusProxy();
    void timeUpdate();

    void clickButton();
    void hideWindow();

private:
    IAppletImport *m_import;

    CalendarButton *m_calendarButton;
    Window *m_window;

    QTimer *m_timeUpdateTimer;
    //DBus 监控时间设置
    QDBusInterface *m_timeDbusProxy;
    //接收DBus时间设置信号
    bool m_isSecondsShowing;
    qint32 m_dateLongFormat;
    qint32 m_dateShortFormat;
    qint32 m_hourFormat;
};

class Plugin : public QObject, public IPlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID IAPPLET_IID FILE "calendar.json")
    Q_INTERFACES(Kiran::IPlugin)

public:
    virtual QWidget *createApplet(const QString &appletID, IAppletImport *import)
    {
        return new Applet(import);
    }
};
}  // namespace Calendar
}  // namespace Kiran
