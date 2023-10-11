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
#include <QWidget>

class QBoxLayout;

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
    virtual int getSize();
    virtual int getOrientation();

private:
    void init();
    void initSelf();
    void initChildren();
    int orientationStr2Enum(const QString& orientation);

private:
    ProfilePanel* m_profilePanel;
    QBoxLayout* m_appletsLayout;

    int m_orientation;
};
}  // namespace Kiran