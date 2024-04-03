
#include <KWindowSystem>
#include <QApplication>
#include <QFocusEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QMouseEvent>
#include <QPainter>
#include <QProcess>
#include <QPushButton>
#include <QSpinBox>
#include <QTableView>
#include <QTextCharFormat>

#include "window.h"

namespace Kiran
{
namespace Calendar
{
Window::Window()
{
    setMouseTracking(true);
    resize(360, 400);

    //将QCalendarWidget原来的导航栏隐藏
    setNavigationBarVisible(false);
    setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    setSelectionMode(QCalendarWidget::SingleSelection);
    setWindowFlags(Qt::WindowStaysOnTopHint);

    //设置星期几中文环境为单字简写，英文环境为普通样式
    QLocale locale;
    if (locale.language() == QLocale::Chinese)
    {
        setLocale(QLocale(QLocale::Chinese));
        setHorizontalHeaderFormat(QCalendarWidget::SingleLetterDayNames);
    }

    //添加星期几
    QTextCharFormat format;
    format.setFontPointSize(9);
    format.setFontWeight(QFont::Medium);
    setWeekdayTextFormat(Qt::Saturday, format);
    setWeekdayTextFormat(Qt::Wednesday, format);
    setWeekdayTextFormat(Qt::Tuesday, format);
    setWeekdayTextFormat(Qt::Monday, format);
    setWeekdayTextFormat(Qt::Thursday, format);
    setWeekdayTextFormat(Qt::Sunday, format);
    setWeekdayTextFormat(Qt::Friday, format);

    //加载导航栏
    initDateChangeWidget();
    initTopSettingWidget();

    //事件过滤器
    installEventFilter(this);
}

Window::~Window()
{
}

void Window::showEvent(QShowEvent *event)
{
    QDate date = QDate::currentDate();
    QLocale locale;
    if (date.year() >= 1970 && date.year() <= 2099 && locale.language() == QLocale::Chinese)
    {
        QString strLunarYear = Lunar::getLunarYearStr(date.year());
        QString strLunar = Lunar::getLunarMonDayStr(date.year(), date.month(), date.day());
        m_lunarYear->setText(strLunarYear);
        m_lunarDay->setText(strLunar);
    }
    else
    {
        QString curDateStr = date.toString("yyyy-MM-dd");
        m_lunarYear->setText(curDateStr);
    }
    KWindowSystem::setState(winId(), NET::SkipTaskbar | NET::SkipPager | NET::SkipSwitcher);
    gotoToday();
}

void Window::paintCell(QPainter *painter, const QRect &rect, const QDate &date) const
{
    // NOTE:使用了主题，选择日期之后，有一个颜色，去不掉，需要主题修复
    painter->save();

    if (date == QDate::currentDate())
    {
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(rect);
        QColor customColor(85, 138, 250);
        QBrush customBrush(customColor);
        painter->setBrush(customBrush);
        painter->drawRoundedRect(rect.adjusted(4, 4, -4, -4), 3, 3);
        painter->restore();
    }

    //字体颜色
    bool isCurrentMonth = (date.month() == monthShown());
    if (!isCurrentMonth)
    {
        painter->setPen(Qt::darkGray);
    }
    //当年份在1970年和2099年之间且语言环境是中文时加载农历，否则只加载公历
    QLocale locale;
    if (date.year() >= 1970 && date.year() <= 2099 && locale.language() == QLocale::Chinese)
    {
        //农历
        QString strLunar = Lunar::getLunarDayStr(date.year(), date.month(), date.day());
        QString strDate = QString::number(date.day()) + "\n" + tr("%1").arg(strLunar);
        painter->drawText(rect, Qt::AlignCenter, strDate);
    }
    else
    {
        painter->drawText(rect, Qt::AlignCenter, QString::number(date.day()));
    }

    painter->restore();
}

bool Window::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::WindowDeactivate)
    {
        emit windowDeactivated();
    }
    return QCalendarWidget::eventFilter(object, event);
}

void Window::initTopSettingWidget()
{
    QWidget *topWidget = new QWidget(this);
    topWidget->setFixedHeight(60);
    topWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    QHBoxLayout *hboxLayout = new QHBoxLayout;
    hboxLayout->setContentsMargins(12, 0, 12, 0);
    hboxLayout->setSpacing(4);

    m_todayBtn = new QPushButton(this);
    m_todayBtn->setText(tr("Today"));
    m_todayBtn->setStyleSheet("QPushButton{ border : none; color : rgb(85, 138, 250)}");

    m_lunarYear = new QLabel(this);
    m_lunarDay = new QLabel(this);
    QLocale locale;
    QDate date = QDate::currentDate();
    if (date.year() >= 1970 && date.year() <= 2099 && locale.language() == QLocale::Chinese)
    {
        QString strLunarYear = Lunar::getLunarYearStr(date.year());
        QString strLunar = Lunar::getLunarMonDayStr(date.year(), date.month(), date.day());
        m_lunarYear->setText(strLunarYear);
        m_lunarDay->setText(strLunar);
    }
    else
    {
        QString curDateStr = date.toString("yyyy-MM-dd");
        m_lunarYear->setText(curDateStr);
    }

    m_settingBtn = new QPushButton(this);
    m_settingBtn->setIcon(QIcon::fromTheme("ks-menu-settings-symbolic"));
    m_settingBtn->setStyleSheet("QPushButton{ border : none;}");

    m_todayBtn->setFixedSize(50, 50);
    m_lunarYear->setFixedSize(80, 50);
    m_lunarDay->setFixedSize(80, 50);
    m_settingBtn->setFixedSize(50, 50);

    hboxLayout->addWidget(m_todayBtn);
    hboxLayout->addStretch(1);
    hboxLayout->addWidget(m_lunarYear);
    hboxLayout->addWidget(m_lunarDay);
    hboxLayout->addStretch(10);
    hboxLayout->addWidget(m_settingBtn);
    topWidget->setLayout(hboxLayout);

    QFont font = this->font();
    font.setPixelSize(18);
    m_lunarYear->setFont(font);
    m_lunarDay->setFont(font);
    m_settingBtn->setFont(font);
    m_todayBtn->setFont(font);

    QVBoxLayout *vBodyLayout = qobject_cast<QVBoxLayout *>(layout());
    vBodyLayout->insertWidget(0, topWidget);
    connect(m_todayBtn, &QPushButton::clicked, this, &Window::gotoToday);
    connect(m_settingBtn, &QPushButton::clicked, this, &Window::settingBtnClicked);
}

void Window::initDateChangeWidget()
{
    QWidget *dayWidget = new QWidget(this);
    dayWidget->setFixedHeight(40);
    dayWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    QHBoxLayout *hboxLayout = new QHBoxLayout(dayWidget);
    hboxLayout->setContentsMargins(12, 0, 12, 0);
    hboxLayout->setSpacing(4);

    QVBoxLayout *vBodyLayout = qobject_cast<QVBoxLayout *>(layout());
    vBodyLayout->insertWidget(0, dayWidget);

    m_subYearBtn = new QPushButton(this);
    m_subYearBtn->setText("<<");
    m_addYearBtn = new QPushButton(this);
    m_addYearBtn->setText(">>");
    m_subMonthBtn = new QPushButton(this);
    m_subMonthBtn->setText("<");
    m_addMonthBtn = new QPushButton(this);
    m_addMonthBtn->setText(">");
    m_yearEdit = new QLineEdit(this);
    QLabel *yearLabel = new QLabel(this);
    m_monthEdit = new QLineEdit(this);
    QLabel *monthLabel = new QLabel(this);

    QLocale locale;
    if (locale.language() == QLocale::Chinese)
    {
        yearLabel->setText("年");
        monthLabel->setText("月");
    }
    else
    {
        yearLabel->setText("-");
    }

    m_subMonthBtn->setFixedSize(25, 25);
    m_addMonthBtn->setFixedSize(25, 25);
    m_subYearBtn->setFixedSize(25, 25);
    m_addYearBtn->setFixedSize(25, 25);
    m_yearEdit->setFixedWidth(45);
    m_monthEdit->setFixedWidth(30);

    hboxLayout->addWidget(m_subYearBtn);
    hboxLayout->addWidget(m_subMonthBtn);
    hboxLayout->addStretch();
    hboxLayout->addWidget(m_yearEdit);
    hboxLayout->addWidget(yearLabel);

    hboxLayout->addWidget(m_monthEdit);
    hboxLayout->addWidget(monthLabel);
    hboxLayout->addStretch();
    hboxLayout->addWidget(m_addMonthBtn);
    hboxLayout->addWidget(m_addYearBtn);

    connect(m_subMonthBtn, &QPushButton::clicked, this, &Window::changeDateTimeBtnClicked);
    connect(m_addMonthBtn, &QPushButton::clicked, this, &Window::changeDateTimeBtnClicked);
    connect(m_subYearBtn, &QPushButton::clicked, this, &Window::changeDateTimeBtnClicked);
    connect(m_addYearBtn, &QPushButton::clicked, this, &Window::changeDateTimeBtnClicked);

    connect(m_yearEdit, &QLineEdit::editingFinished, this, &Window::enterYear);
    connect(m_monthEdit, &QLineEdit::editingFinished, this, &Window::enterMonth);
}
void Window::gotoToday()
{
    setSelectedDate(QDate::currentDate());
    QDate date = QDate::currentDate();
    m_currentYear = date.year();
    m_currentMonth = date.month();
    m_yearEdit->setText(QString::number(m_currentYear));
    m_monthEdit->setText(QString::number(m_currentMonth).rightJustified(2, '0'));
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

void Window::enterYear()
{
    int year = m_yearEdit->text().toInt();
    int month = m_monthEdit->text().toInt();
    m_currentYear = year;
    //超过年份限度自动跳回当前日期
    if (m_currentYear > 2100 || m_currentYear < 1901 || month < 1 || month > 12)
    {
        gotoToday();
        return;
    }
    setCurrentPage(year, month);
    setSelectedDate(QDate(year, month, selectedDate().day()));
}

void Window::enterMonth()
{
    int year = m_yearEdit->text().toInt();
    int month = m_monthEdit->text().toInt();
    m_currentMonth = month;
    //超过年份限度自动跳回当前日期
    if (m_currentYear > 2100 || m_currentYear < 1901 || month < 1 || month > 12)
    {
        gotoToday();
        return;
    }
    setCurrentPage(year, month);
    setSelectedDate(QDate(year, month, selectedDate().day()));
    m_monthEdit->setText(QString::number(month).rightJustified(2, '0'));
}

void Window::changeDateTimeBtnClicked()
{
    QPushButton *senderBtn = qobject_cast<QPushButton *>(sender());
    if (senderBtn == m_subMonthBtn)
    {
        showPreviousMonth();
        if (m_currentMonth > 1)
        {
            m_currentMonth -= 1;
            m_monthEdit->setText(QString::number(m_currentMonth).rightJustified(2, '0'));
        }
        else
        {
            m_currentMonth = 12;
            m_currentYear -= 1;
            m_monthEdit->setText(QString::number(m_currentMonth).rightJustified(2, '0'));
            m_yearEdit->setText(QString::number(m_currentYear));
        }
        setSelectedDate(QDate(m_currentYear, m_currentMonth, selectedDate().day()));
    }
    else if (senderBtn == m_addMonthBtn)
    {
        showNextMonth();
        if (m_currentMonth < 12)
        {
            m_currentMonth += 1;
            m_monthEdit->setText(QString::number(m_currentMonth).rightJustified(2, '0'));
        }
        else
        {
            m_currentMonth = 1;
            m_currentYear += 1;
            m_monthEdit->setText(QString::number(m_currentMonth).rightJustified(2, '0'));
            m_yearEdit->setText(QString::number(m_currentYear));
        }
        setSelectedDate(QDate(m_currentYear, m_currentMonth, selectedDate().day()));
    }
    else if (senderBtn == m_subYearBtn)
    {
        showPreviousYear();
        m_currentYear -= 1;
        m_yearEdit->setText(QString::number(m_currentYear));
        setSelectedDate(QDate(m_currentYear, m_currentMonth, selectedDate().day()));
    }
    else if (senderBtn == m_addYearBtn)
    {
        showNextYear();
        m_currentYear += 1;
        m_yearEdit->setText(QString::number(m_currentYear));
        setSelectedDate(QDate(m_currentYear, m_currentMonth, selectedDate().day()));
    }
    //超过年份限度自动跳回当前日期
    if (m_currentYear > 2100 || m_currentYear < 1901)
    {
        gotoToday();
    }
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
            m_monthEdit->setText(QString::number(m_currentMonth).rightJustified(2, '0'));
        }
        else
        {
            m_currentMonth = 12;
            m_currentYear -= 1;
            m_monthEdit->setText(QString::number(m_currentMonth).rightJustified(2, '0'));
            m_yearEdit->setText(QString::number(m_currentYear));
        }

        setSelectedDate(QDate(m_currentYear, m_currentMonth, selectedDate().day()));
    }
    else if (numSteps < 0)
    {
        if (m_currentMonth < 12)
        {
            m_currentMonth += 1;
            m_monthEdit->setText(QString::number(m_currentMonth).rightJustified(2, '0'));
        }
        else
        {
            m_currentMonth = 1;
            m_currentYear += 1;
            m_monthEdit->setText(QString::number(m_currentMonth).rightJustified(2, '0'));
            m_yearEdit->setText(QString::number(m_currentYear));
        }

        setSelectedDate(QDate(m_currentYear, m_currentMonth, selectedDate().day()));
    }
    //超过年份限度自动跳回当前日期
    if (m_currentYear > 2100 || m_currentYear < 1901)
    {
        gotoToday();
    }

    event->accept();
}

}  // namespace Calendar
}  // namespace Kiran
