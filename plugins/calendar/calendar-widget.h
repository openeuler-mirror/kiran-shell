
#pragma once

#include <QCalendarWidget>

class CalendarWidget : public QCalendarWidget
{
    Q_OBJECT

public:
    CalendarWidget(QWidget *parent = nullptr);
    ~CalendarWidget() override;

protected:
    void paintCell(QPainter *painter, const QRect &rect, const QDate &date) const override;

private:
    QDate m_hoverDate;
};
