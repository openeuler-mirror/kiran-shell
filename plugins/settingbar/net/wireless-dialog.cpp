/**
 * Copyright (c) 2020 ~ 2025 KylinSec Co., Ltd.
 * kiran-shell is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     liuxinhao <liuxinhao@kylinsec.com.cn>
 */

#include "wireless-dialog.h"

#include <QApplication>
#include <QScreen>
#include <QShowEvent>

namespace Kiran
{
namespace SettingBar
{
WirelessDialog::WirelessDialog(DialogType type, QWidget* parent)
    : KiranInputDialog(parent),
      m_type(type)
{
    init();
}

WirelessDialog::~WirelessDialog()
{
}

bool WirelessDialog::getNetworkPasswd(QWidget* w, const QString& desc, const QString& ssid, QString& passwd)
{
    WirelessDialog dialog(WirelessDialog::WIRELESS_DIALOG_TYPE_PASSWD, w);
    dialog.setTitle(tr("WIFI password"));
    dialog.setDesc(desc);
    dialog.setInputMode(QLineEdit::Password, 64);

    if (dialog.exec())
    {
        passwd = dialog.getUserInput();
        return true;
    }
    return false;
}

bool WirelessDialog::getHiddenNetworkSsid(QWidget* w, QString& ssid)
{
    WirelessDialog dialog(WirelessDialog::WIRELESS_DIALOG_TYPE_HIDDEN_SSID, w);
    dialog.setTitle(tr("Hidden wireless network ssid"));
    QString desc = tr("Please enter the hidden wireless network ssid");
    dialog.setDesc(desc);
    dialog.setInputMode(QLineEdit::Normal, 32);

    if (dialog.exec())
    {
        ssid = dialog.getUserInput();
        return true;
    }
    return false;
}

bool WirelessDialog::checkValid(const QString& text)
{
    switch (m_type)
    {
    case WIRELESS_DIALOG_TYPE_PASSWD:
    {
        if (text.size() < 8)
            return false;
        break;
    }
    case WIRELESS_DIALOG_TYPE_HIDDEN_SSID:
    {
        if (text.size() < 1)
            return false;
        break;
    }
    default:
        break;
    }
    return true;
}

void WirelessDialog::init()
{
}

void WirelessDialog::showEvent(QShowEvent* event)
{
    KiranInputDialog::showEvent(event);
    moveTocenterOnScreen();
}

void WirelessDialog::moveTocenterOnScreen()
{
    // 移动到鼠标所在屏幕中心
    QPoint cursorPos = QCursor::pos();
    QScreen* screen = QApplication::screenAt(cursorPos);
    if (screen)
    {
        QRect screenGeometry = screen->geometry();
        QRect dialogRect = frameGeometry();
        int x = screenGeometry.x() + (screenGeometry.width() - dialogRect.width()) / 2;
        int y = screenGeometry.y() + (screenGeometry.height() - dialogRect.height()) / 2;
        move(x, y);
    }
}

}  // namespace SettingBar
}  // namespace Kiran
