/**
 * Copyright (c) 2024 ~ 2025 KylinSec Co., Ltd.
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

#include <kiran-color-block.h>
#include <QBoxLayout>
#include <QWidget>

namespace Kiran
{
class IAppletImport;

namespace SettingBar
{
class Applet;
class SettingWindow;
class SettingButton;
class Window : public KiranColorBlock
{
    Q_OBJECT
public:
    explicit Window(IAppletImport *import, Applet *parent);
    ~Window();

protected:
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void updateLayout();

private:
    Qt::AlignmentFlag getLayoutAlignment();
    QBoxLayout::Direction getLayoutDirection();

    void clickHwConfButton(bool checked);
    void showHwConfWindow();
    void hideHwConfWindow();
    void updateWindowPosition();

signals:

private:
    IAppletImport *m_import;
    QBoxLayout *m_layout;

    QList<SettingButton *> hwConfButtons;
    SettingWindow *m_hwConfWindow;

    bool m_hovered;
    bool m_pressed;
};
}  // namespace SettingBar
}  // namespace Kiran
