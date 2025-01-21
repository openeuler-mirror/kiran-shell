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

#include <QDBusConnection>
#include <QFontMetrics>
#include <QObject>

class QLayout;
class Utility : public QObject
{
    Q_OBJECT
public:
    // 执行命令
    static QByteArray runCmd(QString cmd, QStringList cmdArg = QStringList());
    // 清理布局
    static void clearLayout(QLayout* layout, bool deleteWidget = false, bool hideWidget = false);
    // 获取含省略号的字符串
    static QString getElidedText(QFontMetrics fontMetrics, QString text, int elidedTextLen);
    // 调整弹窗显示位置
    static void updatePopWidgetPos(int panelOriention, QWidget* triggerWidget, QWidget* popWidget);
    // 检查dbus服务是否已注册
    static bool isDbusServiceRegistered(QString serviceName, QDBusConnection::BusType type = QDBusConnection::SessionBus);

private:
    Utility() {}
};
