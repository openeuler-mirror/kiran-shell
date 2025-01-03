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

#include <plugin-i.h>
#include <QBoxLayout>
#include <QWidget>

class QFrame;
class QMenu;
class QGSettings;

namespace Kiran
{
class ProfilePanel;
class ProfileApplet;
class Applet;
class LineFrame;
class Panel : public QWidget, public IPanel
{
    Q_OBJECT

    Q_PROPERTY(QString uid READ getUID)

public:
    Panel(ProfilePanel* profilePanel);

    QString getUID();

public:
    virtual int getSize() Q_DECL_OVERRIDE;
    virtual int getOrientation() Q_DECL_OVERRIDE;

protected:
    void contextMenuEvent(QContextMenuEvent* event) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
    void enterEvent(QEvent* event) Q_DECL_OVERRIDE;
    void leaveEvent(QEvent* event) Q_DECL_OVERRIDE;

private:
    void init();
    void initChildren();
    int orientationStr2Enum(const QString& orientation);
    QString orientationEnum2Str(const int& orientation);

    QScreen* getScreen();
    void updateGeometry(int size = 0);
    void updateLayout();
    QBoxLayout::Direction getLayoutDirection();

    void shellSettingChanged(const QString& key);
    void updatePersonalityMode();

    void updateAutoHide();
    bool isMouseInsideWidgetTree(QWidget* parentWidget);

signals:
    void panelProfileChanged() Q_DECL_OVERRIDE;

private:
    ProfilePanel* m_profilePanel;  // 面板配置
    QBoxLayout* m_appletsLayout;
    QGSettings* m_shellGsettings;

    QList<Applet*> m_applets;

    QList<LineFrame*> m_lineFrames;

    QMenu* m_menu;

    // 显示模式相关
    bool m_isPersonalityMode;
    int m_layoutMargin;
    int m_radius;

    bool m_isAutoHide;
    bool m_isFullShow;
    QTimer* m_leaveDetectTimer;
};
}  // namespace Kiran
