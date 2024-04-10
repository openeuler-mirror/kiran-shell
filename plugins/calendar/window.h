
#pragma once

#include <QCalendarWidget>

#include "lunar.h"

class QPushButton;
class QLineEdit;
class QLabel;

namespace Kiran
{
namespace Calendar
{
class Window : public QCalendarWidget
{
    Q_OBJECT

public:
    Window();
    ~Window();

protected:
    void paintCell(QPainter *painter, const QRect &rect, const QDate &date) const override;
    void wheelEvent(QWheelEvent *event) override;
    void showEvent(QShowEvent *event) override;
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    //顶部设置导航栏加载
    void initTopSettingWidget();
    //日期修改导航栏加载
    void initDateChangeWidget();

private slots:
    //今天按钮操作
    void gotoToday();
    //设置按钮操作
    void settingBtnClicked();
    //输入年份操作
    void enterYear();
    //输入月份操作
    void enterMonth();
    //上/下年月按钮切换操作
    void changeDateTimeBtnClicked();

signals:
    void windowDeactivated();

private:
    //当前页面年月
    int m_currentYear;
    int m_currentMonth;

    //插件
    QPushButton *m_subMonthBtn;
    QPushButton *m_addMonthBtn;
    QPushButton *m_subYearBtn;
    QPushButton *m_addYearBtn;

    QPushButton *m_todayBtn;
    QLabel *m_lunarYear;
    QLabel *m_lunarDay;
    QPushButton *m_settingBtn;

    QLineEdit *m_yearEdit;
    QLineEdit *m_monthEdit;
};
}  // namespace Calendar
}  // namespace Kiran
