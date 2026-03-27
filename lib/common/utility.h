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
class QScreen;
class QPixmap;

class Utility : public QObject
{
    Q_OBJECT
public:
    // 执行命令
    static QByteArray runCmd(QString cmd, QStringList cmdArg = QStringList());
    // 根据 panel 厚度计算 panel 场景下的标准图标与按钮尺寸
    // panelIconSize: 标准图标尺寸（用于 menu/workspace/taskbar 等）
    // panelCompactIconSize: 紧凑图标尺寸（用于 systemtray/settingbar，为标准尺寸的 2/3）
    // panelButtonSize: 按钮尺寸（panel 厚度减去固定边距）
    static int panelIconSize(int panelSize);
    static int panelCompactIconSize(int panelSize);
    static int panelButtonSize(int panelSize);
    // 清理布局
    static void clearLayout(QLayout* layout, bool deleteWidget = false, bool hideWidget = false);
    static void clearLayout(QWidget* widget);
    // 获取含省略号的字符串
    static QString getElidedText(QFontMetrics fontMetrics, QString text, int elidedTextLen);
    // 调整弹窗显示位置
    static void updatePopWidgetPos(QScreen* screen, int panelOriention, QWidget* triggerWidget, QWidget* popWidget);
    // 检查dbus服务是否已注册
    static bool isDbusServiceRegistered(QString serviceName, QDBusConnection::BusType type = QDBusConnection::SessionBus);
    // 获取拼音猜测的汉字
    static QStringList pinyinGuess(const QString& pinyinInput);

    static QPixmap convertOpacity(const QPixmap& source, double opacity);

private:
    Utility();
    ~Utility() override;
};
