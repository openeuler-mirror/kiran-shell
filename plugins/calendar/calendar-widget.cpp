

#include <QHeaderView>
#include <QLocale>
#include <QPainter>
#include <QTableView>
#include <QTextCharFormat>
#include <kiran-integration/theme/palette.h>

#include "calendar-widget.h"
#include "lunar.h"

CalendarWidget::CalendarWidget(QWidget *parent) : QCalendarWidget(parent) {
  // 样式代理
  QTableView *view = findChild<QTableView *>("qt_calendar_calendarview");
  if (view) {
    //        view->setItemDelegate(new ItemDelegate(this));
    //        view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //        view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  }

  setMouseTracking(true);

  //将QCalendarWidget原来的导航栏隐藏
  setNavigationBarVisible(false);
  setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
  setSelectionMode(QCalendarWidget::SingleSelection);
  setWindowFlags(Qt::WindowStaysOnTopHint);

  //设置星期几中文环境为单字简写，英文环境为普通样式
  QLocale locale;
  if (locale.language() == QLocale::Chinese) {
    setLocale(QLocale(QLocale::Chinese));
    setHorizontalHeaderFormat(QCalendarWidget::SingleLetterDayNames);
  }

  //星期表头字体
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

  //    QVBoxLayout *vBodyLayout = qobject_cast<QVBoxLayout *>(layout());
  //    vBodyLayout->setContentsMargins(8, 8, 8, 8);

  //    setStyleSheet("QCalendarWidget QAbstractItemView:selected
  //    {background-color: transparent;}");
}

CalendarWidget::~CalendarWidget() {}

void CalendarWidget::paintCell(QPainter *painter, const QRect &rect,
                               const QDate &date) const {
  //    QCalendarWidget::paintCell(painter, rect, date);
  //    return;
  //    // NOTE:使用了主题，选择日期之后，有一个颜色，去不掉，需要主题修复
  //    QCalendarWidget::paintCell(painter, rect, date);

  //    return;
  //    // 自定义选择日期的颜色
  //    if (date == selectedDate())
  //    {
  //        painter->save();
  //        painter->setBrush(Qt::red);                      //
  //        设置选择日期的颜色为红色 painter->drawRect(rect.adjusted(2, 2, -2,
  //        -2));  // 绘制选择日期的矩形框 painter->restore();
  //    }
  //    return;

  painter->setRenderHint(QPainter::Antialiasing,
                         true); // 设置反走样，使边缘平滑

  QFont font = painter->font();
  font.setWeight(QFont::Light);
  font.setPointSize(8);
  painter->setFont(font);

  if (date == QDate::currentDate()) {
    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(85, 138, 250));
    //        painter->drawRoundedRect(rect.adjusted(4, 4, -4, -4), 3, 3);
    //        painter->drawRect(rect);
    painter->drawRoundedRect(rect.adjusted(4, 4, -4, -4), 4, 4);

    painter->restore();
  }

  if (date == selectedDate()) {
    painter->save();
    //        painter->setPen(QColor(85, 138, 250));
    //        painter->setBrush(Qt::NoBrush);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(85, 138, 250));
    painter->drawRoundedRect(rect, 4, 4);

    painter->restore();
  }

  painter->save();

  //字体颜色
  bool isCurrentMonth = (date.month() == monthShown());
  if (!isCurrentMonth) {
    painter->setPen(Qt::darkGray);
  }
  //当年份在1970年和2099年之间且语言环境是中文时加载农历，否则只加载公历
  QLocale locale;
  if (date.year() >= 1970 && date.year() <= 2099 &&
      locale.language() == QLocale::Chinese) {
    //农历
    QString strLunar =
        Lunar::getLunarDayStr(date.year(), date.month(), date.day());
    QString strDate = QString("%1\n%2").arg(date.day()).arg(strLunar);

    painter->drawText(rect, Qt::AlignCenter, strDate);
  } else {
    painter->drawText(rect, Qt::AlignCenter, QString::number(date.day()));
  }

  painter->restore();
}

QSize ItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                             const QModelIndex &index) const {
  QSize size = QStyledItemDelegate::sizeHint(option, index);
  size.setWidth(10);
  size.setHeight(100);
  return size;
}

void ItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const {
  auto palette = Kiran::Theme::Palette::getDefault();

  QColor bgColor;
  // 选中底色
  if (option.state & QStyle::State_Selected) {
    bgColor = palette->getColor(Kiran::Theme::Palette::SELECTED,
                                Kiran::Theme::Palette::WIDGET);
    painter->fillRect(option.rect, bgColor);
  }

  // 悬停
  if (option.state & QStyle::State_MouseOver) {
    bgColor = palette->getColor(Kiran::Theme::Palette::MOUSE_OVER,
                                Kiran::Theme::Palette::WIDGET);
    painter->fillRect(option.rect, bgColor);
  }

  //    QString text = index.data(Qt::DisplayRole).toString();
  //    QRect textRect = option.rect;
  //    //    textRect.adjust(iconRect.right() - baseRect.x() +
  //    ICON_TEXT_MARGIN, 0, 0, 0); painter->drawText(textRect, Qt::AlignCenter,
  //    text);

  QStyledItemDelegate::paint(painter, option, index);
}
