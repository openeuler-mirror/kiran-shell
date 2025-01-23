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

#include "ks-i.h"
#include "setting-item.h"
#include "ui_setting-item.h"

namespace Kiran
{
namespace SettingBar
{
SettingItem::SettingItem(QWidget *parent, bool settingButtonVisible)
    : KiranColorBlock(parent),
      m_ui(new Ui::SettingItem),
      m_isActive(false)
{
    m_ui->setupUi(this);

    m_ui->toolButtonSetting->setIcon(QIcon::fromTheme(KS_ICON_HWCONF_SETTING));
    setSettingButtonVisible(settingButtonVisible);

    connect(m_ui->toolButtonIcon, &QToolButton::clicked, this, &SettingItem::iconClicked);
    connect(m_ui->toolButtonSetting, &QToolButton::clicked, this, &SettingItem::settingClicked);
}

SettingItem::~SettingItem()
{
    delete m_ui;
}

void SettingItem::setIcon(const QIcon &icon)
{
    m_ui->toolButtonIcon->setIcon(icon);
}

void SettingItem::setTooltip(const QString &tooltip)
{
    m_ui->toolButtonIcon->setToolTip(tooltip);
}

void SettingItem::setSettingButtonVisible(const bool &visible)
{
    m_ui->toolButtonSetting->setVisible(visible);
    // NOTE:临时方案，当设置页面可用时，图标不能点击
    m_ui->toolButtonIcon->setEnabled(!visible);
}

void SettingItem::setActive(const bool &isActive)
{
    m_isActive = isActive;
}

void SettingItem::paintEvent(QPaintEvent *event)
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

}  // namespace SettingBar
}  // namespace Kiran
