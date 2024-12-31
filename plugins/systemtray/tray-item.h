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

#include <dbusmenu-qt5/dbusmenuimporter.h>
#include <QMenu>

#include "lib/widgets/styled-button.h"
#include "tray-item-proxy.h"

#define TRAY_ITEM_DATA_

namespace Kiran
{
namespace Systemtray
{
class TrayItem : public StyledButton
{
    Q_OBJECT
public:
    TrayItem(QString service, QString objectPath, QWidget *parent = nullptr);
    ~TrayItem();

    enum Status
    {
        PASSIVE = 0,
        ACTIVE,
        NEEDSATTENTION
    };

    enum IconType
    {
        BASE_ICON = 0,
        ATTENTION_ICON,
        OVERLAY_ICON
    };

    //    // 用于左键点击，获取位置，后续删除
    //    QString getId();

public slots:
    void updateBaseIcon();
    void updateAttentionIcon();
    void updateOverlayIcon();
    void updateToolTip();
    void updateStatus(const QString &status);
    void updataItemMenu();

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    void performRefresh();
    void refresh();

    void init();
    void getIcon(IconType iconType);
    // 叠加overlay icon
    // icon1: 原始图标
    // icon2: overlay icon
    QIcon mergeIcons(const QIcon &icon1, const QIcon &icon2);
    void updateIconShow();

signals:
    void startDrag(TrayItem *);

private:
    bool m_isInit;
    // dbus路径
    QString m_service;
    QString m_objectPath;

    // dbus属性获取
    TrayItemProxy *m_trayItemProxy;

    // 为什么要定时器去刷新数据：
    // 此进程是服务端
    // 其他应用发送注册信号RegisterStatusNotifierItem触发类的创建，此时该应用正在执行这个信号等待返回
    // 如果马上调用其提供的接口，必然导致堵塞，结果无法返回，此时双方都在等待执行结果
    // 等dbus接口超时后，才会正常
    // 其他情况类似（如果对方需要等待信号的结果，而服务端在处理这个信号时调用了对方的接口）
    QTimer *m_refreshTimer;

    // 状态
    Status m_status;
    // 图标
    QString m_iconThemePath;
    QIcon m_icon;
    QIcon m_overlayIcon;
    QIcon m_attentionIcon;

    // 右键菜单
    DBusMenuImporter *m_dBusMenuImporter;

    // 右键拖动起始位置，用于防止误触，当移动坐标达到阈值之后才判定为拖拽
    QPoint m_pressPoint;

    // 为了兼容以前的自研程序（音量、网络）
    // FIXME: 以前的自定义协议左键是按照写死的id去得到托盘按钮位置，无法支持多个面板
    //     QString m_customId;
};
}  // namespace Systemtray
}  // namespace Kiran
