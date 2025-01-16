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
#include <plugin-i.h>
#include <QDialog>
#include <QGridLayout>
#include <QList>

#include "tray-item.h"

// 托盘区的弹出窗口

namespace Kiran
{
namespace Systemtray
{
class WindowPopup : public QDialog
{
    Q_OBJECT

public:
    WindowPopup(IAppletImport *import, QWidget *parent = nullptr);
    ~WindowPopup();

    // 用于左键点击，获取位置，后续删除
    QList<TrayItem *> getTrayItems();

    // 是否托盘弹出窗口保存的项
    bool isWindowPopupItem(const QString &serviceAndPath);

public slots:
    // 拖动到托盘区结束
    void dropEnd(QString serviceAndPath);

    // 原来保存在traybox中的托盘项，注册时，直接放到traybox
    void AddItem(QString serviceAndPath);

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    void hideEvent(QHideEvent *event) override;

private:
    // 拖拽相关，计算位置
    int getInsertIndex(const QPoint &pos);
    void calculateRowCol(const int &totalSize, int &row, int &col);
    void updateDragPos(const QPoint &pos);

    // 托盘项位置更新
    void updateItemLayout();

    //添加 移除托盘项
    TrayItem *itemAdd(QString serviceAndPath);
    void itemRemove(const QString &serviceAndPath);

    // 保存到配置
    void addWindowPopupItem(const QString &serviceAndPath);
    void removeWindowPopupItem(const QString &serviceAndPath);
    QString getStatusNotifierItemId(const QString &serviceAndPath);

signals:
    void hideTrayBox();
    void dropEnded(QString serviceAndPath);
    void updatePosition();

private:
    IAppletImport *m_import;

    QGridLayout *m_layout;

    // 拖拽相关
    int m_currentDropIndex;
    StyledButton *m_indicatorWidget;
    QList<QWidget *> m_items;

    // 托盘项，显示在托盘区弹出窗口里
    QMap<QString, TrayItem *> m_services;
};
}  // namespace Systemtray
}  // namespace Kiran
