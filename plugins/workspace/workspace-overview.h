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

#include <QMap>
#include <QWidget>

class QScrollArea;
class QVBoxLayout;
namespace Kiran
{
class WindowThumbnail;

namespace Workspace
{
// 工作区预览,包含工作区所管理的应用窗口
class WorkspaceOverview : public QWidget
{
    Q_OBJECT
public:
    explicit WorkspaceOverview(int desktop, QWidget *parent = nullptr);
    ~WorkspaceOverview();

protected:
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateGridLayout();
    void updateWindowItem();

signals:

private:
    int m_workspaceIndex = 0;
    QMap<WId, WindowThumbnail *> m_windows;

    QScrollArea *m_scrollArea;
    QWidget *m_containerWidget;
    QVBoxLayout *m_mainLayout;
};
}  // namespace Workspace
}  // namespace Kiran
