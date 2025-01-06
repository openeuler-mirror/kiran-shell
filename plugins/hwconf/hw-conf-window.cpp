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

#include <qt5-log-i.h>
#include <QKeyEvent>
#include <QProcess>
#include <QTimer>

#include "brightness/brightness.h"
#include "hw-conf-window.h"
#include "net/net-conf-item.h"
#include "theme/theme-conf-item.h"
#include "ui_hw-conf-window.h"

namespace Kiran
{
namespace HwConf
{
HwConfWindow::HwConfWindow(QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint),
      ui(new Ui::HwConfWindow),
      m_isBrightnessPressed(false)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);
    ui->labelBatteryInfo->clear();

    installEventFilter(this);

    // 有线网络
    auto ethernetNetworkItem = new NetConfItem(NetworkManager::Device::Ethernet, this);
    connect(ethernetNetworkItem, &HwConfItem::requestOnlyShow, this, &HwConfWindow::onlyShow);
    connect(ethernetNetworkItem, &HwConfItem::requestExitOnlyShow, this, &HwConfWindow::exitOnlyShow);
    connect(ethernetNetworkItem, &NetConfItem::enableNetwork, [ethernetNetworkItem](bool enabled)
            {
                ethernetNetworkItem->setVisible(enabled);
            });

    // 无线网络
    auto wifiNetworkItem = new NetConfItem(NetworkManager::Device::Wifi, this);
    connect(wifiNetworkItem, &HwConfItem::requestOnlyShow, this, &HwConfWindow::onlyShow);
    connect(wifiNetworkItem, &HwConfItem::requestExitOnlyShow, this, &HwConfWindow::exitOnlyShow);
    connect(wifiNetworkItem, &NetConfItem::enableNetwork, [wifiNetworkItem](bool enabled)
            {
                wifiNetworkItem->setVisible(enabled);
            });

    // 主题
    auto themeItem = new ThemeConfItem(this);

    // 亮度
    auto brightness = new Brightness(this);
    connect(brightness, &Brightness::enableBrightness, [this](bool enabled)
            {
                ui->widgetBrightness->setVisible(enabled);
            });
    connect(brightness, &Brightness::brightnessValueChanged, [this](int value)
            {
                // 当界面主动设置时，禁用来自dbus的被动更新
                if (m_isBrightnessPressed)
                {
                    return;
                }
                KLOG_INFO() << "brightnessValueChanged" << m_isBrightnessPressed;
                ui->sliderBrightness->blockSignals(true);
                ui->sliderBrightness->setValue(value);
                ui->sliderBrightness->blockSignals(false);
                ui->widgetBrightness->setVisible(true);
                updateBrightnessIcon();
            });
    connect(brightness, &Brightness::isReadyToUpdate, [this](bool &isReadyToUpdate)
            {
                // 当界面主动设置时，禁用来自dbus的被动更新
                isReadyToUpdate = !m_isBrightnessPressed;
            });

    connect(ui->sliderBrightness, &QSlider::sliderPressed, [this]()
            {
                m_isBrightnessPressed = true;
                KLOG_INFO() << "sliderPressed" << m_isBrightnessPressed;
            });
    connect(ui->sliderBrightness, &QSlider::sliderReleased, [this]()
            {
                m_isBrightnessPressed = false;
                KLOG_INFO() << "sliderReleased" << m_isBrightnessPressed;
            });

    connect(this, &HwConfWindow::setBrightness, brightness, &Brightness::setBrightness, Qt::QueuedConnection);

    m_changeBrightnessTimer = new QTimer(this);
    m_changeBrightnessTimer->setSingleShot(true);
    m_changeBrightnessTimer->setInterval(300);
    connect(m_changeBrightnessTimer, &QTimer::timeout, [this]()
            {
                emit setBrightness(m_curBrightnessValue);
            });

    // 组件初始化
    ethernetNetworkItem->init();
    wifiNetworkItem->init();
    brightness->init();

    if (!NetCommon::hasEthernetDevices())
    {
        ethernetNetworkItem->setVisible(false);
    }
    if (!NetCommon::hasWifiDevices())
    {
        wifiNetworkItem->setVisible(false);
    }

    ui->m_layoutItems->addWidget(ethernetNetworkItem, 0, 0, Qt::AlignLeft | Qt::AlignTop);
    ui->m_layoutItems->addWidget(wifiNetworkItem, 0, 1, Qt::AlignLeft | Qt::AlignTop);
    ui->m_layoutItems->addWidget(themeItem, 0, 2, Qt::AlignLeft | Qt::AlignTop);
    auto horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    ui->m_layoutItems->addItem(horizontalSpacer, 0, 3);
    // 使窗口缩小到最小，由控件撑大到最合适的大小
    resize(360, 1);
}

HwConfWindow::~HwConfWindow()
{
    delete ui;
}

void HwConfWindow::syncVolumeValue(const int &value)
{
    ui->sliderVolume->blockSignals(true);
    ui->sliderVolume->setValue(value);
    ui->sliderVolume->blockSignals(false);
}

void HwConfWindow::syncVolumeIcon(const QIcon &icon)
{
    ui->toolButtonVolume->setIcon(icon);
}

void HwConfWindow::syncVolumeEnabled(const bool &enabled)
{
    ui->widgetVolume->setVisible(enabled);
}

void HwConfWindow::syncBatteryValue(const QString &value)
{
    if (!value.isEmpty())
    {
        ui->labelBatteryInfo->setText(value + "%");
    }
}

void HwConfWindow::syncBatteryIcon(const QIcon &icon)
{
    ui->toolButtonBattery->setIcon(icon);
}

void HwConfWindow::syncBatteryEnabled(const bool &enabled)
{
    ui->widgetBattery->setVisible(enabled);
}

bool HwConfWindow::eventFilter(QObject *object, QEvent *event)
{
    // window was deactivated
    if (QEvent::WindowDeactivate == event->type())
    {
        emit windowDeactivated();
    }

    return QWidget::eventFilter(object, event);
}

void HwConfWindow::keyPressEvent(QKeyEvent *event)
{
    if (Qt::Key_Escape == event->key())
    {
        emit windowDeactivated();
    }
    else
    {
        QWidget::keyPressEvent(event);
    }
}

void HwConfWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
}

void HwConfWindow::onlyShow(QWidget *widget)
{
    while (QLayoutItem *item = ui->layoutPageSub->takeAt(0))
    {
        if (QWidget *widget = item->widget())
        {
            widget->setVisible(false);
        }
        delete item;
    }

    ui->layoutPageSub->addWidget(widget);
    widget->setVisible(true);
    ui->stackedWidget->setCurrentIndex(1);

    resize(width(), 400);
    emit updatePosition();
}

void HwConfWindow::exitOnlyShow()
{
    ui->stackedWidget->setCurrentIndex(0);

    resize(width(), 1);
    emit updatePosition();
}

void HwConfWindow::updateBrightnessIcon()
{
    int value = ui->sliderBrightness->value();
    int iconLevel = value / 10 - 2;  // 图标只有1-8档亮度
    if (iconLevel < 1)
    {
        iconLevel = 1;
    }
    if (iconLevel > 8)
    {
        iconLevel = 8;
    }

    ui->toolButtonBrightness->setIcon(QIcon::fromTheme(QString("ksvg-ks-brightness-%1").arg(iconLevel)));
}

void HwConfWindow::on_toolButtonVolume_clicked()
{
    emit setVolumeMute();
}
void HwConfWindow::on_sliderVolume_valueChanged(int value)
{
    emit setVolume(value);
}

void HwConfWindow::on_sliderBrightness_valueChanged(int value)
{
    m_changeBrightnessTimer->start();
    m_curBrightnessValue = value;
    updateBrightnessIcon();
}

void HwConfWindow::on_toolButtonVolumeSetting_clicked()
{
    QProcess::startDetached("kiran-control-panel", {"-c", "audio"});
}

void HwConfWindow::on_toolButtonBrightness_clicked()
{
    QProcess::startDetached("kiran-control-panel", {});
}

void HwConfWindow::on_toolButtonBattery_clicked()
{
    QProcess::startDetached("kiran-control-panel", {});
}

void HwConfWindow::on_toolButtonSettingPanel_clicked()
{
    QProcess::startDetached("kiran-control-panel", {});
}

}  // namespace HwConf
}  // namespace Kiran
