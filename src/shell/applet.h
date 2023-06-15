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
#include <QSharedPointer>
#include <QWidget>

namespace Kiran
{
namespace Model
{
class Applet;
}

class AppletArgs : public IAppletArgs
{
};

// class Applet : public QObject
// {
//     Q_OBJECT

// public:
//     Applet(const QString &uid, QObject *parent = nullptr);

//     // 获取插件并构建控件
//     QWidget *build();

//     QString getUID() { return this->m_uid; };
//     QString getID() { return this->m_id; };
//     Panel *getPanel();

//     void setID(const QString &id) { this->m_id = id; };

//     // bool isValid() { return this->m_iApplet != nullptr; }
//     // QWidget *widget() { return this->m_iApplet->widget(); }

// private:
//     QString m_uid;
//     QString m_id;
// };
}  // namespace Kiran