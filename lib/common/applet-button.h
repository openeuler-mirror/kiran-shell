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

#include <QPushButton>

namespace Kiran
{
class IAppletImport;

class AppletButton : public QPushButton
{
    Q_OBJECT

public:
    AppletButton(IAppletImport *import);
    virtual ~AppletButton();

protected:
    // 通过名称设置图标，图标大小会根据按钮大小做调整
    void setIconByName(const QString &iconName);
    void setIconFromTheme(const QString &iconName);

private:
    IAppletImport *m_import;
};

}  // namespace Kiran
