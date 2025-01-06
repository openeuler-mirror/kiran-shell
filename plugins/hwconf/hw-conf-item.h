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
#include <QWidget>

namespace Ui
{
class HwConfItem;
}

namespace Kiran
{
namespace HwConf
{
class HwConfItem : public KiranColorBlock
{
    Q_OBJECT

public:
    explicit HwConfItem(QWidget *parent = nullptr, bool settingButtonVisible = true);
    virtual ~HwConfItem();

    void setIcon(const QIcon &icon);
    void setTooltip(const QString &tooltip);
    void setSettingButtonVisible(const bool &visible);
    void setActive(const bool &isActive);

private:
    void paintEvent(QPaintEvent *event) override;

signals:
    void iconClicked();
    void settingClicked();
    void requestOnlyShow(QWidget *widget);
    void requestExitOnlyShow();

private:
    Ui::HwConfItem *ui;

    // 连接标志
    bool m_isActive;
};
}  // namespace HwConf
}  // namespace Kiran
