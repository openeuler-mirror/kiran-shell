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
 * Author:     yangfeng <yangfeng@kylinsec.com.cn>
 */

#include <kiran-integration/theme/palette.h>
#include <QDate>
#include <QPainter>
#include <QProcess>
#include <QWheelEvent>

#include "ks-i.h"
#include "lunar.h"
#include "ui_window.h"
#include "window.h"

namespace Kiran
{
namespace Calendar
{
Window::Window(QWidget *parent)
    : QDialog(parent, Qt::WindowFlags() | Qt::FramelessWindowHint),
      m_ui(new Ui::Window)
{
    m_ui->setupUi(this);

    setAttribute(Qt::WA_TranslucentBackground);

    // 事件过滤器
    installEventFilter(this);

    initTopSettingWidget();
    initDateChangeWidget();

    connect(m_ui->calendarWidget, &QCalendarWidget::selectionChanged, [this]()
            {
                QDate date = m_ui->calendarWidget->selectedDate();
                showDate(date);
            });
}

Window::~Window()
{
    delete m_ui;
}

void Window::wheelEvent(QWheelEvent *event)
{
    int numDegrees = event->angleDelta().y() / 8;
    int numSteps = numDegrees / 15;
    if (numSteps > 0)
    {
        if (m_currentMonth > 1)
        {
            m_currentMonth -= 1;
            m_ui->monthSpinBox->setValue(m_currentMonth);
        }
        else
        {
            m_currentMonth = 12;
            m_currentYear -= 1;
            m_ui->monthSpinBox->setValue(m_currentMonth);
            m_ui->yearSpinBox->setValue(m_currentYear);
        }

        m_ui->calendarWidget->setSelectedDate(QDate(m_currentYear, m_currentMonth, m_ui->calendarWidget->selectedDate().day()));
    }
    else if (numSteps < 0)
    {
        if (m_currentMonth < 12)
        {
            m_currentMonth += 1;
            m_ui->monthSpinBox->setValue(m_currentMonth);
        }
        else
        {
            m_currentMonth = 1;
            m_currentYear += 1;
            m_ui->monthSpinBox->setValue(m_currentMonth);
            m_ui->yearSpinBox->setValue(m_currentYear);
        }

        m_ui->calendarWidget->setSelectedDate(QDate(m_currentYear, m_currentMonth, m_ui->calendarWidget->selectedDate().day()));
    }
    // 超过年份限度自动跳回当前日期
    if (m_currentYear > 2100 || m_currentYear < 1901)
    {
        gotoToday();
    }

    event->accept();
}

void Window::showEvent(QShowEvent *event)
{
    gotoToday();
}

bool Window::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::WindowDeactivate)
    {
        emit windowDeactivated();
    }
    return QDialog::eventFilter(object, event);
}

void Window::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    auto palette = Kiran::Theme::Palette::getDefault();
    QColor bgColor = palette->getColor(Kiran::Theme::Palette::NORMAL, Kiran::Theme::Palette::WINDOW);

    painter.setBrush(bgColor);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect(), 4, 4);
}

void Window::initTopSettingWidget()
{
    m_ui->todayBtn->setCheckable(false);
    m_ui->todayBtn->setText(tr("Today"));

    auto palette = Kiran::Theme::Palette::getDefault();
    auto bgColor = palette->getColor(Kiran::Theme::Palette::SELECTED,
                                     Kiran::Theme::Palette::WIDGET);
    m_ui->todayBtn->setTextColor(bgColor);

    m_ui->settingBtn->setCheckable(false);
    m_ui->settingBtn->setText("");
    m_ui->settingBtn->setIcon(QIcon::fromTheme(KS_ICON_MENU_SETTINGS_SYMBOLIC));

    QFont font = this->font();
    font.setPixelSize(18);
    font.setWeight(QFont::Light);
    m_ui->lunarYear->setFont(font);
    m_ui->lunarDay->setFont(font);
    m_ui->settingBtn->setFont(font);
    m_ui->todayBtn->setFont(font);

    connect(m_ui->todayBtn, &QAbstractButton::clicked, this, &Window::gotoToday);
    connect(m_ui->settingBtn, &QToolButton::clicked, this, &Window::settingBtnClicked);

    showDate(QDate::currentDate());
}

void Window::initDateChangeWidget()
{
    QLocale locale;
    if (locale.language() == QLocale::Chinese)
    {
        m_ui->yearLabel->setText("年");
        m_ui->monthLabel->setText("月");
    }
    else
    {
        m_ui->yearLabel->setText("-");
        m_ui->monthLabel->setText("");
    }

    connect(m_ui->subMonthBtn, &QAbstractButton::clicked, this, &Window::changeDateTimeBtnClicked);
    connect(m_ui->addMonthBtn, &QAbstractButton::clicked, this, &Window::changeDateTimeBtnClicked);
    connect(m_ui->subYearBtn, &QAbstractButton::clicked, this, &Window::changeDateTimeBtnClicked);
    connect(m_ui->addYearBtn, &QAbstractButton::clicked, this, &Window::changeDateTimeBtnClicked);
    connect(m_ui->yearSpinBox, SIGNAL(valueChanged(int)), this, SLOT(enterYear()));
    connect(m_ui->monthSpinBox, SIGNAL(valueChanged(int)), this, SLOT(enterMonth()));
}

void Window::settingBtnClicked()
{
    QString program = "kiran-control-panel";
    QStringList arguments;
    arguments << "-c"
              << "timedate";
    QProcess::startDetached(program, arguments);
    hide();
}

void Window::gotoToday()
{
    QDate date = QDate::currentDate();
    m_currentYear = date.year();
    m_currentMonth = date.month();
    m_ui->yearSpinBox->setValue(m_currentYear);
    m_ui->monthSpinBox->setValue(m_currentMonth);
    m_ui->calendarWidget->setSelectedDate(QDate::currentDate());

    showDate(date);
}

void Window::enterYear()
{
    int year = m_ui->yearSpinBox->value();
    int month = m_ui->monthSpinBox->value();
    m_currentYear = year;
    // 超过年份限度自动跳回当前日期
    if (m_currentYear > 2100 || m_currentYear < 1901 || month < 1 || month > 12)
    {
        gotoToday();
        return;
    }
    m_ui->calendarWidget->setCurrentPage(year, month);
    m_ui->calendarWidget->setSelectedDate(QDate(year, month, m_ui->calendarWidget->selectedDate().day()));
}

void Window::enterMonth()
{
    int year = m_ui->yearSpinBox->value();
    int month = m_ui->monthSpinBox->value();
    m_currentMonth = month;
    // 超过年份限度自动跳回当前日期
    if (m_currentYear > 2100 || m_currentYear < 1901 || month < 1 || month > 12)
    {
        gotoToday();
        return;
    }
    m_ui->calendarWidget->setCurrentPage(year, month);
    m_ui->calendarWidget->setSelectedDate(QDate(year, month, m_ui->calendarWidget->selectedDate().day()));
    m_ui->monthSpinBox->setValue(month);
}

void Window::changeDateTimeBtnClicked()
{
    StyledButton *senderBtn = qobject_cast<StyledButton *>(sender());
    if (senderBtn == m_ui->subMonthBtn)
    {
        m_ui->calendarWidget->showPreviousMonth();
        if (m_currentMonth > 1)
        {
            m_currentMonth -= 1;
            m_ui->monthSpinBox->setValue(m_currentMonth);
        }
        else
        {
            m_currentMonth = 12;
            m_currentYear -= 1;
            m_ui->monthSpinBox->setValue(m_currentMonth);
            m_ui->yearSpinBox->setValue(m_currentYear);
        }
        m_ui->calendarWidget->setSelectedDate(QDate(m_currentYear, m_currentMonth, m_ui->calendarWidget->selectedDate().day()));
    }
    else if (senderBtn == m_ui->addMonthBtn)
    {
        m_ui->calendarWidget->showNextMonth();
        if (m_currentMonth < 12)
        {
            m_currentMonth += 1;
            m_ui->monthSpinBox->setValue(m_currentMonth);
        }
        else
        {
            m_currentMonth = 1;
            m_currentYear += 1;
            m_ui->monthSpinBox->setValue(m_currentMonth);
            m_ui->yearSpinBox->setValue(m_currentYear);
        }
        m_ui->calendarWidget->setSelectedDate(QDate(m_currentYear, m_currentMonth, m_ui->calendarWidget->selectedDate().day()));
    }
    else if (senderBtn == m_ui->subYearBtn)
    {
        m_ui->calendarWidget->showPreviousYear();
        m_currentYear -= 1;
        m_ui->yearSpinBox->setValue(m_currentYear);
        m_ui->calendarWidget->setSelectedDate(QDate(m_currentYear, m_currentMonth, m_ui->calendarWidget->selectedDate().day()));
    }
    else if (senderBtn == m_ui->addYearBtn)
    {
        m_ui->calendarWidget->showNextYear();
        m_currentYear += 1;
        m_ui->yearSpinBox->setValue(m_currentYear);
        m_ui->calendarWidget->setSelectedDate(QDate(m_currentYear, m_currentMonth, m_ui->calendarWidget->selectedDate().day()));
    }
    // 超过年份限度自动跳回当前日期
    if (m_currentYear > 2100 || m_currentYear < 1901)
    {
        gotoToday();
    }
}

void Window::showDate(QDate date)
{
    QLocale locale;
    if (date.year() >= 1970 && date.year() <= 2099 && locale.language() == QLocale::Chinese)
    {
        QString strLunarYear = Lunar::getLunarYearStr(date.year(), date.month(), date.day());
        QString strLunar = Lunar::getLunarMonDayStr(date.year(), date.month(), date.day());
        QString lunarDayExtraName = Lunar::getLunarDayStr(date.year(), date.month(), date.day());
        if (!strLunar.contains(lunarDayExtraName))
        {
            strLunar += " " + lunarDayExtraName;
        }

        m_ui->lunarYear->setText(strLunarYear);
        m_ui->lunarDay->setText(strLunar);
    }
    else
    {
        QString curDateStr = date.toString("yyyy-MM-dd");
        m_ui->lunarYear->setText(curDateStr);
        m_ui->lunarDay->clear();
    }
}
}  // namespace Calendar
}  // namespace Kiran
