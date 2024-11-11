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

#include <QObject>

#include "lib/widgets/styled-button.h"

namespace Kiran
{
namespace Workspace
{
// 创建工作区按钮的重绘
class WorkspaceAddBtn : public StyledButton
{
    Q_OBJECT
public:
    WorkspaceAddBtn(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
};
}  // namespace Workspace
}  // namespace Kiran
