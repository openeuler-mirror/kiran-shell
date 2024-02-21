/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
 * kiran-session-manager is licensed under Mulan PSL v2.
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
#include <QBoxLayout>
#include <QMap>
#include <QWidget>

#include "window-previewer.h"

namespace Kiran
{
class IAppletImport;

namespace Taskbar
{
class AppPreviewer : public QWidget
{
    Q_OBJECT
public:
    explicit AppPreviewer(IAppletImport *import, QWidget *parent = nullptr);

    // 关联KWindowSystem，增加或关闭窗口
    void addWindow(WId wid);
    void removeWindow(WId wid);

private:
    // 获取panel方向信息
    QBoxLayout::Direction getLayoutDirection();
    Qt::AlignmentFlag getLayoutAlignment();
    // 布局更新
    void updateLayout();

protected:
    void showEvent(QShowEvent *event);
    void leaveEvent(QEvent *event);

signals:
    void closeWindow(WId wid);

private:
    QBoxLayout *m_layout;

    IAppletImport *m_import;

    QMap<int, WindowPreviewer *> m_mapWindowPreviewers;  // key: wid
};

}  // namespace Taskbar

}  // namespace Kiran
