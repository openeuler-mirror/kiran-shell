/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
 * kiran-shell is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan
 * PSL v2. You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 * NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE. See the
 * Mulan PSL v2 for more details.
 *
 * Author:     yanglan <yanglan@kylinsec.com.cn>
 */

#include <qt5-log-i.h>
#include <QCoreApplication>
#include <QFont>
#include <QGridLayout>
#include <QTimer>
#include <QTranslator>

#include "applet.h"
#include "calendar-button.h"
#include "ks-config.h"
#include "ks_timedate_interface.h"
#include "lib/common/dbus-service-watcher.h"
#include "lib/common/logging-category.h"
#include "lib/common/utility.h"
#include "window.h"

#define KIRAN_TIMEDATA_BUS "com.kylinsec.Kiran.SystemDaemon.TimeDate"
#define KIRAN_TIMEDATA_PATH "/com/kylinsec/Kiran/SystemDaemon/TimeDate"
#define PROPERTIES_INTERFACE "org.freedesktop.DBus.Properties"
#define PROPERTIES_CHANGED "PropertiesChanged"

namespace Kiran
{
namespace Calendar
{
Applet::Applet(IAppletImport *import)
    : m_import(import),
      m_ksTimeDate(nullptr),
      m_isSecondsShowing(true),
      m_dateLongFormat(0),
      m_dateShortFormat(0),
      m_hourFormat(0)
{
    initTranslator();
    initPanelConnections();
    initUI();
    initDBusConnections();

    initTimer();
}

void Applet::initTranslator()
{
    static QTranslator translator;
    if (!translator.load(QLocale(), "calendar", ".", KS_INSTALL_TRANSLATIONDIR,
                         ".qm"))
    {
        KLOG_INFO(LCCalendar) << "Load translator failed!";
    }
    else
    {
        QCoreApplication::installTranslator(&translator);
    }
}

void Applet::initPanelConnections()
{
    QObject *panelObject = dynamic_cast<QObject *>(m_import->getPanel());
    if (panelObject)
    {
        connect(panelObject, SIGNAL(panelProfileChanged()), this, SLOT(updateLayout()));
    }
    else
    {
        KLOG_INFO(LCCalendar) << "Failed to cast panel object!";
    }
}

void Applet::initUI()
{
    m_window = new Window(this);
    m_window->hide();
    connect(m_window, &Window::windowDeactivated, this, &Applet::hideWindow);

    QFont font = this->font();
    font.setPixelSize(10);
    setFont(font);

    setRadius(0);

    m_calendarButton = new CalendarButton(m_import, this);
    connect(m_calendarButton, &QPushButton::clicked, this, &Applet::clickButton);

    QGridLayout *layout = new QGridLayout(this);
    layout->setMargin(4);
    layout->setSpacing(0);
    layout->addWidget(m_calendarButton);
}

void Applet::initDBusConnections()
{
    QDBusConnection::systemBus().connect(
        KIRAN_TIMEDATA_BUS, KIRAN_TIMEDATA_PATH, PROPERTIES_INTERFACE,
        PROPERTIES_CHANGED, this, SLOT(timeInfoChanged()));

    connect(&DBusWatcher, &DBusServiceWatcher::serviceOwnerChanged, this, &Applet::serviceOwnerChanged);
    DBusWatcher.AddService(KIRAN_TIMEDATA_BUS, QDBusConnection::SystemBus);

    if (Utility::isDbusServiceRegistered(KIRAN_TIMEDATA_BUS, QDBusConnection::SystemBus))
    {
        initTimeDbusProxy();
    }
    else
    {
        KLOG_INFO(LCCalendar) << "dbus service not registered:" << KIRAN_TIMEDATA_BUS;
    }
}

void Applet::initTimer()
{
    m_timeUpdateTimer = new QTimer(this);
    connect(m_timeUpdateTimer, &QTimer::timeout, this, &Applet::timeUpdate);
    m_timeUpdateTimer->start(1000);
}

void Applet::serviceOwnerChanged(const QString &service, const QString &oldOwner, const QString &newOwner)
{
    if (KIRAN_TIMEDATA_BUS != service)
    {
        return;
    }
    if (oldOwner.isEmpty())
    {
        KLOG_INFO(LCCalendar) << "dbus service registered:" << service;
        initTimeDbusProxy();
    }
}

void Applet::clickButton()
{
    m_window->setVisible(true);
    m_calendarButton->setEnabled(false);

    auto oriention = m_import->getPanel()->getOrientation();
    Utility::updatePopWidgetPos(oriention, this, m_window);
}

void Applet::hideWindow()
{
    m_window->setVisible(false);
    m_calendarButton->setChecked(false);
    m_calendarButton->setEnabled(true);
}

void Applet::initTimeDbusProxy()
{
    if (m_ksTimeDate)
    {
        m_ksTimeDate->deleteLater();
        m_ksTimeDate = nullptr;
    }
    // If the remote service service is not present or if an error occurs trying to obtain the description of the remote interface interface,
    // the object created will not be valid
    m_ksTimeDate = new KSTimeDate(
        KIRAN_TIMEDATA_BUS, KIRAN_TIMEDATA_PATH,
        QDBusConnection::systemBus(), this);
    timeInfoChanged();
}

void Applet::timeInfoChanged()
{
    if (!m_ksTimeDate || !m_ksTimeDate->isValid())
    {
        return;
    }

    m_isSecondsShowing = m_ksTimeDate->seconds_showing();
    m_dateLongFormat = m_ksTimeDate->date_long_format_index();
    m_dateShortFormat = m_ksTimeDate->date_short_format_index();
    m_hourFormat = m_ksTimeDate->hour_format();

    timeUpdate();
}

void Applet::updateLayout()
{
    // 根据位置调高
    auto size = m_import->getPanel()->getSize();
    auto oriention = m_import->getPanel()->getOrientation();
    switch (oriention)
    {
    case PanelOrientation::PANEL_ORIENTATION_TOP:
    case PanelOrientation::PANEL_ORIENTATION_BOTTOM:
        setFixedSize(size * 3, size);
        break;
    case PanelOrientation::PANEL_ORIENTATION_RIGHT:
    case PanelOrientation::PANEL_ORIENTATION_LEFT:
        setFixedSize(size, size * 3);
        break;
    default:
        KLOG_INFO(LCCalendar) << "Unknown oriention " << oriention;
        break;
    }

    timeUpdate();
}

void Applet::timeUpdate()
{
    auto oriention = m_import->getPanel()->getOrientation();

    QDateTime curDateTime = QDateTime::currentDateTime();

    QString timeStr = curDateTime.toString("A hh:mm:ss");
    if (!m_isSecondsShowing)
    {
        timeStr = curDateTime.toString("A hh:mm");
    }
    if (m_hourFormat)
    {
        timeStr = curDateTime.toString("hh:mm:ss");
        if (!m_isSecondsShowing)
        {
            timeStr = curDateTime.toString("hh:mm");
        }
    }

    QString dateStr = curDateTime.toString("yyyy/MM/dd");
    QString dateWeekStr =
        curDateTime.toString("hh:mm") + "\n" + curDateTime.toString("dddd");
    QString dateTimeStr = dateWeekStr + "\n" + curDateTime.toString("M/dd");
    if (m_dateShortFormat == 1)
    {
        dateStr = curDateTime.toString("yyyy.MM.dd");
        dateTimeStr = dateWeekStr + "\n" + curDateTime.toString("M.dd");
    }
    else if (m_dateShortFormat == 2)
    {
        dateStr = curDateTime.toString("yyyy-MM-dd");
        dateTimeStr = dateWeekStr + "\n" + curDateTime.toString("M-dd");
    }

    // 按钮显示内容，根据位置不同显示内容不同
    switch (oriention)
    {
    case PanelOrientation::PANEL_ORIENTATION_TOP:
        m_calendarButton->setText(timeStr + "\n" + dateStr);
        break;
    case PanelOrientation::PANEL_ORIENTATION_RIGHT:
        m_calendarButton->setText(dateTimeStr);
        break;
    case PanelOrientation::PANEL_ORIENTATION_BOTTOM:
        m_calendarButton->setText(timeStr + "\n" + dateStr);
        break;
    case PanelOrientation::PANEL_ORIENTATION_LEFT:
        m_calendarButton->setText(dateTimeStr);
        break;
    default:
        KLOG_INFO(LCCalendar) << "Unknown oriention " << oriention;
        break;
    }

    // 提示信息显示内容
    QString curYearDateStr = curDateTime.toString("yyyy");
    QString curWeekDateStr = curDateTime.toString("dddd");
    QString curMonthDateStr = curDateTime.toString("MM");
    QString curDayDateStr = curDateTime.toString("dd");
    QString yearStr = "/";
    QString monthStr = "/";
    QString dayStr = "";
    if (QLocale().language() == QLocale::Chinese)
    {
        // 仅中文环境下显示，不需要翻译
        yearStr = "年";
        monthStr = "月";
        dayStr = "日";
    }

    QString tooltip;
    switch (m_dateLongFormat)
    {
    case 1:
        tooltip = QString("%1%2%3%4%5%6,%7")
                      .arg(curYearDateStr)
                      .arg(yearStr)
                      .arg(curMonthDateStr)
                      .arg(monthStr)
                      .arg(curDayDateStr)
                      .arg(dayStr)
                      .arg(curWeekDateStr);
        break;
    case 2:
        tooltip = QString("%1%2%3%4%5%6")
                      .arg(curYearDateStr)
                      .arg(yearStr)
                      .arg(curMonthDateStr)
                      .arg(monthStr)
                      .arg(curDayDateStr)
                      .arg(dayStr);
        break;
    case 3:
        tooltip = QString("%1/%2/%3,%4")
                      .arg(curYearDateStr)
                      .arg(curMonthDateStr)
                      .arg(curDayDateStr)
                      .arg(curWeekDateStr);
        break;
    case 4:
        tooltip = QString("%1,%2/%3/%4")
                      .arg(curWeekDateStr)
                      .arg(curYearDateStr)
                      .arg(curMonthDateStr)
                      .arg(curDayDateStr);
        break;
    case 5:
        tooltip = QString("%1-%2-%3,%4")
                      .arg(curYearDateStr)
                      .arg(curMonthDateStr)
                      .arg(curDayDateStr)
                      .arg(curWeekDateStr);
        break;
    case 6:
        tooltip = QString("%1,%2-%3-%4")
                      .arg(curWeekDateStr)
                      .arg(curYearDateStr)
                      .arg(curMonthDateStr)
                      .arg(curDayDateStr);
        break;
    case 7:
        tooltip = QString("%1.%2.%3,%4")
                      .arg(curYearDateStr)
                      .arg(curMonthDateStr)
                      .arg(curDayDateStr)
                      .arg(curWeekDateStr);
        break;
    case 8:
        tooltip = QString("%1,%2.%3.%4")
                      .arg(curWeekDateStr)
                      .arg(curYearDateStr)
                      .arg(curMonthDateStr)
                      .arg(curDayDateStr);
        break;
    default:
        tooltip = QString("%1,%2%3%4%5%6%7")
                      .arg(curWeekDateStr)
                      .arg(curYearDateStr)
                      .arg(yearStr)
                      .arg(curMonthDateStr)
                      .arg(monthStr)
                      .arg(curDayDateStr)
                      .arg(dayStr);
    }
    m_calendarButton->setToolTip(tooltip);
}

Applet::~Applet() {}

}  // namespace Calendar
}  // namespace Kiran
