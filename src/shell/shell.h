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

#include <QMap>
#include <QString>
#include "panel.h"

namespace Kiran
{
class Panel;

class Shell : public QObject
{
    Q_OBJECT
public:
    static Shell* getInstance()
    {
        return m_instance;
    };

    static void globalInit();
    static void globalDeinit();

public:
    Panel* getPanel(const QString& uid);

private:
    Shell();
    void init();
    void initChildren();

private:
    static Shell* m_instance;

    QMap<QString, Panel*> m_panels;
};
}  // namespace Kiran
