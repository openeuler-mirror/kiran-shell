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

#include <QObject>

#include "statusnotifieriteminterface.h"

class TrayItemProxy : public QObject
{
    Q_OBJECT
public:
    explicit TrayItemProxy(const QString &service, const QString &path, QObject *parent = nullptr);

    QDBusVariant getProperty(QString const &name);
    QString service() const;

public slots:
    QDBusPendingReply<> activate(int x, int y);
    QDBusPendingReply<> contextMenu(int x, int y);
    QDBusPendingReply<> scroll(int delta, const QString &orientation);
    QDBusPendingReply<> secondaryActivate(int x, int y);

signals:
    void updateAttentionIcon();
    void updateIcon();
    void updateOverlayIcon();
    void updateStatus(const QString &status);
    void updateTitle();
    void updateToolTip();

private:
    org::kde::StatusNotifierItem m_statusNotifierItem;
};
