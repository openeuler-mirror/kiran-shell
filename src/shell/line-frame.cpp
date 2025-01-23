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

#include <QSizePolicy>

#include "line-frame.h"
#include "ui_line-frame.h"

namespace Kiran
{
LineFrame::LineFrame(QWidget *parent)
    : KiranColorBlock(parent),
      m_ui(new Ui::LineFrame)
{
    m_ui->setupUi(this);
    setRadius(0);
}

LineFrame::~LineFrame()
{
    delete m_ui;
}

void LineFrame::setFrameShape(QFrame::Shape type)
{
    m_ui->line->setFrameShape(type);
    if (QFrame::HLine == type)
    {
        m_ui->gridLayout->setContentsMargins(10, 0, 10, 0);
    }
    else if (QFrame::VLine == type)
    {
        m_ui->gridLayout->setContentsMargins(0, 10, 0, 10);
    }
}
}  // namespace Kiran
