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
#include "lib/common/logging-category.h"
#include "net/net-conf-item.h"
#include "setting-window.h"
#include "theme/theme-conf-item.h"
#include "ui_setting-window.h"

namespace Kiran
{
namespace SettingBar
{
SettingWindow::SettingWindow(QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint),
      m_ui(new Ui::SettingWindow),
      m_isBrightnessPressed(false)
{
    m_ui->setupUi(this);
    m_ui->stackedWidget->setCurrentIndex(0);
    m_ui->labelBatteryInfo->clear();

    installEventFilter(this);

    // 有线网络
    auto ethernetNetworkItem = new NetConfItem(NetworkManager::Device::Ethernet, this);
    connect(ethernetNetworkItem, &SettingItem::requestOnlyShow, this, &SettingWindow::onlyShow);
    connect(ethernetNetworkItem, &SettingItem::requestExitOnlyShow, this, &SettingWindow::exitOnlyShow);
    connect(ethernetNetworkItem, &NetConfItem::enableNetwork, [ethernetNetworkItem](bool enabled)
            {
                ethernetNetworkItem->setVisible(enabled);
            });

    // 无线网络
    auto wifiNetworkItem = new NetConfItem(NetworkManager::Device::Wifi, this);
    connect(wifiNetworkItem, &SettingItem::requestOnlyShow, this, &SettingWindow::onlyShow);
    connect(wifiNetworkItem, &SettingItem::requestExitOnlyShow, this, &SettingWindow::exitOnlyShow);
    connect(wifiNetworkItem, &NetConfItem::enableNetwork, [wifiNetworkItem](bool enabled)
            {
                wifiNetworkItem->setVisible(enabled);
            });

    // 主题
    auto themeItem = new ThemeConfItem(this);
    connect(themeItem, &ThemeConfItem::enableTheme, this, [themeItem](bool enabled)
            {
                themeItem->setVisible(enabled);
            });

    // 亮度
    auto brightness = new Brightness(this);
    connect(brightness, &Brightness::enableBrightness, [this](bool enabled)
            {
                m_ui->widgetBrightness->setVisible(enabled);
            });
    connect(brightness, &Brightness::brightnessValueChanged, [this](int value)
            {
                // 当界面主动设置时，禁用来自dbus的被动更新
                if (m_isBrightnessPressed)
                {
                    return;
                }
                KLOG_INFO(LCSettingbar) << "brightnessValueChanged" << m_isBrightnessPressed;
                m_ui->sliderBrightness->blockSignals(true);
                m_ui->sliderBrightness->setValue(value);
                m_ui->sliderBrightness->blockSignals(false);
                m_ui->widgetBrightness->setVisible(true);
                updateBrightnessIcon();
            });
    connect(brightness, &Brightness::isReadyToUpdate, [this](bool &isReadyToUpdate)
            {
                // 当界面主动设置时，禁用来自dbus的被动更新
                isReadyToUpdate = !m_isBrightnessPressed;
            });

    connect(m_ui->sliderBrightness, &QSlider::sliderPressed, [this]()
            {
                m_isBrightnessPressed = true;
                KLOG_INFO(LCSettingbar) << "sliderPressed" << m_isBrightnessPressed;
            });
    connect(m_ui->sliderBrightness, &QSlider::sliderReleased, [this]()
            {
                m_isBrightnessPressed = false;
                KLOG_INFO(LCSettingbar) << "sliderReleased" << m_isBrightnessPressed;
            });

    connect(this, &SettingWindow::setBrightness, brightness, &Brightness::setBrightness, Qt::QueuedConnection);

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
    themeItem->init();

    if (!NetCommon::hasEthernetDevices())
    {
        ethernetNetworkItem->setVisible(false);
    }
    if (!NetCommon::hasWifiDevices())
    {
        wifiNetworkItem->setVisible(false);
    }

    m_ui->layoutItems->addWidget(ethernetNetworkItem, 0, 0, Qt::AlignLeft | Qt::AlignTop);
    m_ui->layoutItems->addWidget(wifiNetworkItem, 0, 1, Qt::AlignLeft | Qt::AlignTop);
    m_ui->layoutItems->addWidget(themeItem, 0, 2, Qt::AlignLeft | Qt::AlignTop);
    auto horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_ui->layoutItems->addItem(horizontalSpacer, 0, 3);
    // 使窗口缩小到最小，由控件撑大到最合适的大小
    resize(360, 1);
}

SettingWindow::~SettingWindow()
{
    delete m_ui;
}

void SettingWindow::syncVolumeValue(const int &value)
{
    m_ui->sliderVolume->blockSignals(true);
    m_ui->sliderVolume->setValue(value);
    m_ui->sliderVolume->blockSignals(false);
}

void SettingWindow::syncVolumeIcon(const QIcon &icon)
{
    m_ui->toolButtonVolume->setIcon(icon);
}

void SettingWindow::syncVolumeEnabled(const bool &enabled)
{
    m_ui->widgetVolume->setVisible(enabled);
}

void SettingWindow::syncBatteryValue(const QString &value)
{
    if (!value.isEmpty())
    {
        m_ui->labelBatteryInfo->setText(value + "%");
    }
}

void SettingWindow::syncBatteryIcon(const QIcon &icon)
{
    m_ui->toolButtonBattery->setIcon(icon);
}

void SettingWindow::syncBatteryEnabled(const bool &enabled)
{
    m_ui->widgetBattery->setVisible(enabled);
}

bool SettingWindow::eventFilter(QObject *object, QEvent *event)
{
    // window was deactivated
    if (QEvent::WindowDeactivate == event->type())
    {
        emit windowDeactivated();
    }

    return QWidget::eventFilter(object, event);
}

void SettingWindow::keyPressEvent(QKeyEvent *event)
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

void SettingWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
}

void SettingWindow::onlyShow(QWidget *widget)
{
    while (QLayoutItem *item = m_ui->layoutPageSub->takeAt(0))
    {
        if (QWidget *widget = item->widget())
        {
            widget->setVisible(false);
        }
        delete item;
    }

    m_ui->layoutPageSub->addWidget(widget);
    widget->setVisible(true);
    m_ui->stackedWidget->setCurrentIndex(1);

    resize(width(), 400);
    emit updatePosition();
}

void SettingWindow::exitOnlyShow()
{
    m_ui->stackedWidget->setCurrentIndex(0);

    resize(width(), 1);
    emit updatePosition();
}

void SettingWindow::updateBrightnessIcon()
{
    int value = m_ui->sliderBrightness->value();
    int iconLevel = value / 10 - 2;  // 图标只有1-8档亮度
    if (iconLevel < 1)
    {
        iconLevel = 1;
    }
    if (iconLevel > 8)
    {
        iconLevel = 8;
    }

    m_ui->toolButtonBrightness->setIcon(QIcon::fromTheme(QString("ksvg-ks-brightness-%1").arg(iconLevel)));
}

void SettingWindow::on_toolButtonVolume_clicked()
{
    emit setVolumeMute();
}
void SettingWindow::on_sliderVolume_valueChanged(int value)
{
    emit setVolume(value);
}

void SettingWindow::on_sliderBrightness_valueChanged(int value)
{
    m_changeBrightnessTimer->start();
    m_curBrightnessValue = value;
    updateBrightnessIcon();
}

void SettingWindow::on_toolButtonVolumeSetting_clicked()
{
    KLOG_INFO(LCSettingbar) << "start detached: kiran-control-panel -c audio";
    QProcess::startDetached("kiran-control-panel", {"-c", "audio"});
}

void SettingWindow::on_toolButtonBrightness_clicked()
{
    KLOG_INFO(LCSettingbar) << "start detached: kiran-control-panel";
    QProcess::startDetached("kiran-control-panel", {});
}

void SettingWindow::on_toolButtonBattery_clicked()
{
    KLOG_INFO(LCSettingbar) << "start detached: kiran-control-panel";
    QProcess::startDetached("kiran-control-panel", {});
}

void SettingWindow::on_toolButtonSettingPanel_clicked()
{
    KLOG_INFO(LCSettingbar) << "start detached: kiran-control-panel";
    QProcess::startDetached("kiran-control-panel", {});
}

}  // namespace SettingBar
}  // namespace Kiran
