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

#include <kiran-color-block.h>
#include <QBoxLayout>
#include <QWidget>

#include "lib/widgets/styled-button.h"
#include "plugin-i.h"
#include "status_notifier_watcher_interface.h"
#include "tray-extended.h"

namespace Kiran
{
namespace Systemtray
{
class TrayItem;

class Tray : public KiranColorBlock
{
    Q_OBJECT
public:
    explicit Tray(IAppletImport *import, QWidget *parent = nullptr);
    virtual ~Tray();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    // 获取panel方向信息
    QBoxLayout::Direction getLayoutDirection();
    Qt::AlignmentFlag getLayoutAlignment();

    // 托盘项注册与注销
    void getRegisteredItems();
    void statusNotifierItemRegister(const QString &serviceAndPath);
    void statusNotifierItemUnregister(const QString &serviceAndPath);
    TrayItem *itemAdd(QString serviceAndPath);
    void itemRemove(const QString &serviceAndPath);

    // 扩展托盘弹出
    void startUpdateTrayExtendedPos();
    void updateTrayExtendedPos();
    void hideTrayExtended();

    // 拖拽到扩展托盘结束
    void dropEnd(QString serviceAndPath);

    // 拖拽相关，计算位置
    int getInsertedIndex(const QPoint &pos);
    // 托盘项位置更新
    void updateItemLayout();

private slots:
    // panel布局信息发生变化
    void updateLayout();

signals:
    void dropEnded(QString serviceAndPath);

private:
    IAppletImport *m_import;
    // 第一层布局，放置扩展托盘窗口和第二层布局
    QBoxLayout *m_layoutBase;
    // 第二层布局，放置各个托盘项
    QBoxLayout *m_layout;

    // 托盘项
    QMap<QString, TrayItem *> m_services;

    // 扩展托盘
    TrayExtended *m_trayExtendedWindow;
    StyledButton *m_windowPopupButton;
    bool m_updateWindowPopupPosInProgress;

    // 拖拽相关
    int m_currentDropIndex;
    StyledButton *m_indicatorWidget;

    // 托盘项，显示在主托盘里
    QList<QWidget *> m_items;

    // 托盘服务监控
    StatusNotifierWatcherInterface *m_statusNotifierWatcherInterface;

    // 本显示服务名称
    QString m_statusNotifierHostName;
};
}  // namespace Systemtray
}  // namespace Kiran
