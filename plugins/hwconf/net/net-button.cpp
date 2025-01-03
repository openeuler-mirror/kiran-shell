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
#include <NetworkManagerQt/ConnectionSettings>
#include <NetworkManagerQt/Device>
#include <QContextMenuEvent>
#include <QDBusReply>
#include <QMenu>
#include <QProcess>

#include "net-button.h"
#include "net-common.h"

namespace Kiran
{
namespace HwConf
{
NetButton::NetButton(QWidget* parent)
    : HwConfButton(parent)
{
    connect(&NetCommonInstance, &NetCommon::netStatusChanged, this, &NetButton::updateNetworkStatus);
    updateNetworkStatus();
}

void NetButton::updateNetworkStatus()
{
    QPair<QString, QString> iconWithTooltip = NetCommon::getNetworkIcon();
    setIcon(QIcon::fromTheme(iconWithTooltip.first));
    setToolTip(iconWithTooltip.second);
}

void NetButton::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu menu;
    menu.addAction(tr("Network settings"), this, [this]()
                   {
                       QProcess::startDetached("kiran-control-panel", {"-c", "network"});
                   });

    menu.exec(mapToGlobal(event->pos()));
    update();
}
}  // namespace HwConf
}  // namespace Kiran
