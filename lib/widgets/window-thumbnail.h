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

#include <KWindowInfo>
#include <QWidget>

namespace Ui
{
class WindowThumbnail;
}

namespace Kiran
{
class WindowThumbnail : public QWidget
{
    Q_OBJECT

public:
    explicit WindowThumbnail(WId wid, QWidget *parent = nullptr);
    virtual ~WindowThumbnail();

    // 获取实际应该的大小,包含可以缩放的部分和额外不能缩放的部分
    void getOriginalSize(int &scaleWidth, int &scaleHeight, int &extraWidth, int &extraHeight);

private slots:
    virtual void on_m_btnClose_clicked();

protected:
    // 监测窗口变化
    void changedWindow(WId wid, NET::Properties properties, NET::Properties2 properties2);

    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

    void refresh();
    void updateVisualName();

protected:
    Ui::WindowThumbnail *m_ui;

    // 应用窗口id
    WId m_wid;

    // 鼠标悬浮标志
    bool m_isHover;
};
}  // namespace Kiran
