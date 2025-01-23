/**
 * Copyright (c) 2024 ~ 2025 KylinSec Co., Ltd.
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

#include <kiran-color-block.h>
#include <QFrame>

namespace Ui
{
class LineFrame;
}

namespace Kiran
{
class LineFrame : public KiranColorBlock
{
    Q_OBJECT

public:
    explicit LineFrame(QWidget *parent = nullptr);
    ~LineFrame();

    void setFrameShape(QFrame::Shape type);

private:
    Ui::LineFrame *m_ui;
};
}  // namespace Kiran
