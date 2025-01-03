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

#include <QDialog>

namespace Ui
{
class HwConfWindow;
}
namespace Kiran
{
namespace HwConf
{
class HwConfWindow : public QDialog
{
    Q_OBJECT

public:
    explicit HwConfWindow(QWidget* parent = nullptr);
    ~HwConfWindow();

    void syncVolumeValue(const int& value);
    void syncVolumeIcon(const QIcon& icon);
    void syncVolumeEnabled(const bool& enabled);

    void syncBatteryValue(const QString& value);
    void syncBatteryIcon(const QIcon& icon);
    void syncBatteryEnabled(const bool& enabled);

protected:
    bool eventFilter(QObject* object, QEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    void onlyShow(QWidget* widget);
    void exitOnlyShow();

    void updateBrightnessIcon();

private slots:
    void on_sliderVolume_valueChanged(int value);
    void on_toolButtonVolume_clicked();
    void on_toolButtonVolumeSetting_clicked();
    void on_sliderBrightness_valueChanged(int value);
    void on_toolButtonBrightness_clicked();
    void on_toolButtonBattery_clicked();
    void on_toolButtonSettingPanel_clicked();

signals:
    void windowDeactivated();
    void updatePosition();
    void setVolume(int value);
    void setVolumeMute();
    void setBrightness(int value);

private:
    Ui::HwConfWindow* ui;

    // 界面调整时，阻止界面被动更新
    bool m_isBrightnessPressed;
    // 设置亮度的接口反应很慢，如果连续多次调用，会导致dbus返回值丢失
    // 滑动时通过定时器触发
    QTimer* m_changeBrightnessTimer;
    int m_curBrightnessValue;
};
}  // namespace HwConf
}  // namespace Kiran
