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

#include <QWidget>

namespace Ui
{
class WorkspaceThumbnail;
}

namespace Kiran
{
namespace Workspace
{
// 工作区缩略图
class WorkspaceThumbnail : public QWidget
{
    Q_OBJECT

public:
    explicit WorkspaceThumbnail(int desktop, QWidget *parent = nullptr);
    ~WorkspaceThumbnail();

    void updateContent();

private slots:
    void on_btnClose_clicked();

protected:
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    void getDesktopBackground();

signals:
    void removeDesktop(int desktop);

private:
    Ui::WorkspaceThumbnail *m_ui;

    int m_workspaceIndex = 0;

    QPixmap m_desktopBackground;

    // 鼠标悬浮标志
    bool m_isHover;
};
}  // namespace Workspace
}  // namespace Kiran
