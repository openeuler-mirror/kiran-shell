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

#include <QWidget>

class QBoxLayout;

namespace Kiran
{
namespace Model
{
class Panel;
class Applet;
}  // namespace Model

class Panel : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QString uid READ getUID)
public:
    enum class Orientation
    {
        ORIENTATION_TOP,
        ORIENTATION_RIGHT,
        ORIENTATION_BOTTOM,
        ORIENTATION_LEFT
    };

public:
    Panel(QSharedPointer<Model::Panel> panelModel);
    QString getUID();

private:
    void initUI();
    void initSelf();
    void initChildren();

    QWidget* createApplet(QSharedPointer<Model::Applet> appletModel);

    Orientation orientationStr2Enum(const QString& orientation);

private:
    QSharedPointer<Model::Panel> m_panelModel;
    QBoxLayout* m_appletsLayout;

    Orientation m_orientation;
};
}  // namespace Kiran