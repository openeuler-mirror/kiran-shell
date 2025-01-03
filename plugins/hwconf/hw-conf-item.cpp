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

#include <kiran-color-block.h>
#include <kiran-integration/theme/palette.h>
#include <QPainter>

#include "hw-conf-item.h"
#include "ks-i.h"
#include "ui_hw-conf-item.h"

namespace Kiran
{
namespace HwConf
{
HwConfItem::HwConfItem(QWidget *parent, bool settingButtonVisible)
    : KiranColorBlock(parent),
      ui(new Ui::HwConfItem),
      m_isActive(false)
{
    ui->setupUi(this);

    ui->toolButtonSetting->setIcon(QIcon::fromTheme(KS_ICON_HWCONF_SETTING));
    setSettingButtonVisible(settingButtonVisible);

    connect(ui->toolButtonIcon, &QToolButton::clicked, this, &HwConfItem::iconClicked);
    connect(ui->toolButtonSetting, &QToolButton::clicked, this, &HwConfItem::settingClicked);
}

HwConfItem::~HwConfItem()
{
    delete ui;
}

void HwConfItem::setIcon(const QIcon &icon)
{
    ui->toolButtonIcon->setIcon(icon);
}

void HwConfItem::setTooltip(const QString &tooltip)
{
    ui->toolButtonIcon->setToolTip(tooltip);
}

void HwConfItem::setSettingButtonVisible(const bool &visible)
{
    ui->toolButtonSetting->setVisible(visible);
    // NOTE:临时方案，当设置页面可用时，图标不能点击
    ui->toolButtonIcon->setEnabled(!visible);
}

void HwConfItem::setActive(const bool &isActive)
{
    m_isActive = isActive;
}

void HwConfItem::paintEvent(QPaintEvent *event)
{
    KiranColorBlock::paintEvent(event);

    //    QPainter painter(this);
    //    painter.setRenderHint(QPainter::Antialiasing, true);

    //    auto palette = Kiran::Theme::Palette::getDefault();

    //    // 背景绘制
    //    QColor bgColor;
    //    if (m_isActive)
    //    {
    //        bgColor = palette->getColor(Kiran::Theme::Palette::ACTIVE, Kiran::Theme::Palette::WIDGET);
    //    }
    //    else
    //    {
    //        bgColor = Qt::transparent;
    //    }

    //    painter.setBrush(bgColor);
    //    painter.setPen(Qt::NoPen);
    //    painter.drawRect(rect());
}

}  // namespace HwConf
}  // namespace Kiran
