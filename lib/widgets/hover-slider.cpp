/**
 * Copyright (c) 2025 ~ 2026 KylinSec Co., Ltd.
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

#include <QEvent>
#include <QStyleOptionSlider>
#include <QWidget>

#include "hover-slider.h"

HoverSlider::HoverSlider(QWidget *parent, Qt::Orientation orientation)
    : QSlider(orientation, parent)
{
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover, true);

    m_tooltip = new QLabel(nullptr, Qt::ToolTip | Qt::FramelessWindowHint);
    m_tooltip->setAttribute(Qt::WA_TranslucentBackground);
    m_tooltip->setAttribute(Qt::WA_TransparentForMouseEvents);
}

HoverSlider::~HoverSlider()
{
    delete m_tooltip;
}

void HoverSlider::mouseMoveEvent(QMouseEvent *event)
{
    showValueLabel();
    QSlider::mouseMoveEvent(event);
}

bool HoverSlider::event(QEvent *event)
{
    if (event->type() == QEvent::HoverMove)
    {
        showValueLabel();
    }
    else if (event->type() == QEvent::HoverLeave)
    {
        m_tooltip->hide();
    }
    return QSlider::event(event);
}

void HoverSlider::showValueLabel()
{
    QStyleOptionSlider opt;
    initStyleOption(&opt);
    QRect handle = style()->subControlRect(QStyle::CC_Slider, &opt,
                                           QStyle::SC_SliderHandle, this);

    QString text = QString::number(value());
    m_tooltip->setText(text);
    m_tooltip->adjustSize();

    QSize tipSize = m_tooltip->sizeHint();

    // 默认显示在手柄上方
    QPoint tipPos = mapToGlobal(QPoint(handle.center().x() - tipSize.width() / 2, handle.top() - tipSize.height()));

    m_tooltip->move(tipPos);
    m_tooltip->show();
}
