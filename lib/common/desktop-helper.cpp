/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
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

#include <KWindowSystem/NETWM>
#include <KWindowSystem>
#include <QMap>
#include <QtX11Extras/QX11Info>

#include "desktop-helper.h"

DesktopHelper::DesktopHelper(QObject *parent)
    : QObject{parent}
{
    connect(KWindowSystem::self(), &KWindowSystem::currentDesktopChanged, this, &DesktopHelper::currentDesktopChanged);
    connect(KWindowSystem::self(), &KWindowSystem::numberOfDesktopsChanged, this, &DesktopHelper::numberOfDesktopsChanged);
}

DesktopHelper &DesktopHelper::getInstance()
{
    static DesktopHelper instance;
    return instance;
}

int DesktopHelper::numberOfDesktops()
{
    return KWindowSystem::numberOfDesktops();
}

int DesktopHelper::currentDesktop()
{
    return KWindowSystem::currentDesktop();
}

void DesktopHelper::setCurrentDesktop(int desktop)
{
    KWindowSystem::setCurrentDesktop(desktop);
}

void DesktopHelper::createDesktop()
{
    NETRootInfo info(QX11Info::connection(), NET::NumberOfDesktops);
    // 增加桌面数
    info.setNumberOfDesktops(numberOfDesktops() + 1);
}

void DesktopHelper::removeDesktop(int deskToRemove)
{
    // 由于无法指定删除哪个桌面
    // 所以需要将需要删除的桌面内部的窗口前移,后面桌面逐个前移
    // 直到最后一个桌面清空
    // 这种情况下减少桌面,减少的就是最后一个空桌面
    int numOfDesk = numberOfDesktops();
    if (numOfDesk <= 1 || numOfDesk < deskToRemove || deskToRemove < 1)
    {
        return;
    }

    QMap<int, QList<WId>> winWithDesk;
    for (int i = 1; i <= numOfDesk; i++)
    {
        winWithDesk[i] = QList<WId>{};
    }
    QList<WId> windows = KWindowSystem::windows();
    for (auto window : windows)
    {
        KWindowInfo windowInfo(window, NET::WMDesktop);
        if (windowInfo.valid())
        {
            winWithDesk[windowInfo.desktop()].append(window);
        }
    }

    for (int i = deskToRemove; i <= numOfDesk; i++)
    {
        // 要移除第一个桌面,需要将第二个桌面的窗口移动到第一个
        if (i == 1)
        {
            i++;
            continue;
        }

        // 将此桌面的窗口前移
        for (auto window : winWithDesk[i])
        {
            KWindowSystem::setOnDesktop(window, i - 1);
        }
    }

    NETRootInfo info(QX11Info::connection(), NET::NumberOfDesktops);
    // 减少桌面数
    info.setNumberOfDesktops(info.numberOfDesktops() - 1);
}

void DesktopHelper::moveToDesktop(WId wid, int desktop)
{
    KWindowSystem::setOnDesktop(wid, desktop);
}
