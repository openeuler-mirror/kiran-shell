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

#include <dbusmenu-qt5/dbusmenuimporter.h>
#include <QMenu>
#include <QToolButton>
#include "tray-item-proxy.h"

#define TRAY_ITEM_DATA_

namespace Kiran
{
namespace Systemtray
{
class TrayItem : public QToolButton
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

    // 用于左键点击，获取位置，后续删除
    QString getId();

public slots:
    void updateIcon();
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
    void init();
    void getIcon(IconType iconType);
    // 叠加overlay icon
    // icon1: 原始图标
    // icon2: overlay icon
    QIcon mergeIcons(const QIcon &icon1, const QIcon &icon2);
    void updateIconShow();

signals:
    void startDrag();

private:
    // dbus路径
    QString m_service;
    QString m_objectPath;

    // dbus属性获取
    TrayItemProxy *m_trayItemProxy;

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

    //为了兼容以前的自研程序（音量、网络）
    //FIXME: 以前的自定义协议左键是按照写死的id去得到托盘按钮位置，无法支持多个面板
    QString m_customId;
};
}  // namespace Systemtray
}  // namespace Kiran
