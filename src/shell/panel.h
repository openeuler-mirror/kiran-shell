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
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#pragma once

#include <plugin-i.h>
#include <style-palette.h>
#include <QBoxLayout>
#include <QMenu>
#include <QWidget>

namespace Kiran
{
class ProfilePanel;
class ProfileApplet;

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

public slots:
    void loadStylesheet(Kiran::PaletteType paletteType);

protected:
    void contextMenuEvent(QContextMenuEvent* event) Q_DECL_OVERRIDE;

private:
    void init();
    void initChildren();
    int orientationStr2Enum(const QString& orientation);
    QString orientationEnum2Str(const int& orientation);

    QScreen* getScreen();
    void updateShow();
    void updateGeometry();
    void updateLayout();
    QBoxLayout::Direction getLayoutDirection();

signals:
    void panelProfileChanged() Q_DECL_OVERRIDE;

private:
    ProfilePanel* m_profilePanel;  //面板配置
    QBoxLayout* m_appletsLayout;
};
}  // namespace Kiran
