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
#pragma once

#include <QDialog>

namespace Ui
{
class Window;
}

namespace Kiran
{
namespace Calendar
{
class Window : public QDialog
{
    Q_OBJECT

public:
    explicit Window(QWidget *parent = nullptr);
    ~Window();

protected:
    void wheelEvent(QWheelEvent *event) override;
    void showEvent(QShowEvent *event) override;
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    //顶部设置导航栏加载
    void initTopSettingWidget();
    //日期修改导航栏加载
    void initDateChangeWidget();

    //设置按钮操作
    void settingBtnClicked();
    //今天按钮操作
    void gotoToday();

    //输入年份操作
    void enterYear();
    //输入月份操作
    void enterMonth();
    //上/下年月按钮切换操作
    void changeDateTimeBtnClicked();

signals:
    void windowDeactivated();

private:
    Ui::Window *m_ui;

    //当前页面年月
    int m_currentYear;
    int m_currentMonth;
};
}  // namespace Calendar
}  // namespace Kiran
