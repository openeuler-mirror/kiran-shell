

#include <kiran-integration/theme/palette.h>
#include <qt5-log-i.h>
#include <QLocale>
#include <QPainter>
#include <QTextCharFormat>

#include "calendar-widget.h"
#include "lunar.h"

CalendarWidget::CalendarWidget(QWidget *parent)
    : QCalendarWidget(parent)
{
    // 将QCalendarWidget原来的导航栏隐藏
    setNavigationBarVisible(false);
    setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    setSelectionMode(QCalendarWidget::SingleSelection);
    setWindowFlags(Qt::WindowStaysOnTopHint);

    // 设置星期几中文环境为单字简写，英文环境为普通样式
    QLocale locale;
    if (locale.language() == QLocale::Chinese)
    {
        setLocale(QLocale(QLocale::Chinese));
        setHorizontalHeaderFormat(QCalendarWidget::SingleLetterDayNames);
    }

    // 星期表头字体
    QTextCharFormat format;
    format.setFontPointSize(10);
    format.setFontWeight(QFont::Light);
    setWeekdayTextFormat(Qt::Sunday, format);
    setWeekdayTextFormat(Qt::Monday, format);
    setWeekdayTextFormat(Qt::Tuesday, format);
    setWeekdayTextFormat(Qt::Saturday, format);
    setWeekdayTextFormat(Qt::Wednesday, format);
    setWeekdayTextFormat(Qt::Friday, format);
    setWeekdayTextFormat(Qt::Thursday, format);
}

CalendarWidget::~CalendarWidget() = default;

void CalendarWidget::paintCell(QPainter *painter, const QRect &rect,
                               const QDate &date) const
{
    painter->save();

    auto *palette = Kiran::Theme::Palette::getDefault();
    painter->setRenderHint(QPainter::Antialiasing);

    painter->setPen(Qt::NoPen);

    // 清除底色
    auto bgColor = palette->getColor(Kiran::Theme::Palette::NORMAL,
                                     Kiran::Theme::Palette::WINDOW);
    painter->setBrush(bgColor);
    painter->drawRect(rect);

    if (date == QDate::currentDate())  // 当前日期
    {
        auto bgColor = palette->getColor(Kiran::Theme::Palette::SELECTED,
                                         Kiran::Theme::Palette::WIDGET);
        painter->setBrush(bgColor);
    }
    else if (date == selectedDate())  // 选中日期
    {
        auto bgColor = palette->getColor(Kiran::Theme::Palette::SUNKEN,
                                         Kiran::Theme::Palette::WIDGET);
        painter->setBrush(bgColor);
    }
    else if (rect.contains(mapFromGlobal(QCursor::pos())))  // 鼠标悬浮日期
    {
        auto bgColor = palette->getColor(Kiran::Theme::Palette::MOUSE_OVER,
                                         Kiran::Theme::Palette::WIDGET);
        painter->setBrush(bgColor);
    }

    painter->drawRoundedRect(rect, 4, 4);

    painter->restore();
    painter->save();

    QFont font = painter->font();
    font.setWeight(QFont::Light);
    font.setPointSize(8);
    painter->setFont(font);

    // 字体颜色
    bool isCurrentMonth = (date.month() == monthShown());
    if (!isCurrentMonth)
    {
        painter->setPen(Qt::darkGray);
    }

    // 当年份在1970年和2099年之间且语言环境是中文时加载农历，否则只加载公历
    QLocale locale;
    if (date.year() >= 1970 && date.year() <= 2099 &&
        locale.language() == QLocale::Chinese)
    {
        // 农历
        QString strLunar =
            Lunar::getLunarDayStr(date.year(), date.month(), date.day());
        QString strDate = QString("%1\n%2").arg(date.day()).arg(strLunar);

        painter->drawText(rect, Qt::AlignCenter, strDate);
    }
    else
    {
        painter->drawText(rect, Qt::AlignCenter, QString::number(date.day()));
    }

    painter->restore();
}
