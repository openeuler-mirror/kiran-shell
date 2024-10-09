
#pragma once

#include <QCalendarWidget>
#include <QStyledItemDelegate>

class QPushButton;
class QLineEdit;
class QLabel;
class StyledButton;

class ItemDelegate : public QStyledItemDelegate
{
public:
    ItemDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent) {}

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

class CalendarWidget : public QCalendarWidget
{
    Q_OBJECT

public:
    CalendarWidget(QWidget *parent = nullptr);
    ~CalendarWidget();

protected:
    void paintCell(QPainter *painter, const QRect &rect, const QDate &date) const override;
};
