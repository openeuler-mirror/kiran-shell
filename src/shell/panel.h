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
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#pragma once

#include <QBoxLayout>
#include <QSharedPointer>

#include "lib/common/shell-window.h"
#include "plugin-i.h"

class QFrame;
class QGSettings;

namespace Kiran
{
class ProfilePanel;
class ProfileApplet;
class Applet;
class LineFrame;
class Panel : public ShellWindow, public IPanel
{
    Q_OBJECT

    Q_PROPERTY(QString uid READ getUID)

public:
    Panel(QSharedPointer<ProfilePanel> profilePanel);
    ~Panel() override;

    void init();

    QString getUID();
    int getSize() override;
    int getOrientation() override;
    QScreen* getScreen() override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
    void showEvent(QShowEvent* event) override;
    bool event(QEvent* event) override;

private:
    void initChildren();

    int orientationStr2Enum(const QString& orientation);
    QString orientationEnum2Str(const int& orientation);

    void updateGeometry(int size = 0);
    void updateLayout();
    QBoxLayout::Direction getLayoutDirection();

    void shellSettingChanged(const QString& key);
    void updatePersonalityMode();

    void updateAutoHide();
    bool isMouseInsideWidgetTree(QWidget* parentWidget);

    void connectToCurrentScreen();

signals:
    void panelProfileChanged() override;

private:
    QSharedPointer<ProfilePanel> m_profilePanel;
    QBoxLayout* m_appletsLayout = nullptr;
    QGSettings* m_gsettings = nullptr;

    QStringList m_appletsUID;  // list顺序为插件顺序，用户定义的插件名称，具有唯一性
    QMap<QString, Applet*> m_applets;

    QList<LineFrame*> m_lineFrames;

    // 显示模式相关
    bool m_isPersonalityMode = false;
    int m_layoutMargin = 0;
    int m_radius = 0;

    bool m_isAutoHide = false;
    bool m_isFullShow = false;
    QTimer* m_leaveDetectTimer;
    QMetaObject::Connection m_screenConnection;
};
}  // namespace Kiran
