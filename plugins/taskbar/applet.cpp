/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
 * kiran-session-manager is licensed under Mulan PSL v2.
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

#include <ks-i.h>
#include <plugin-i.h>
#include <qt5-log-i.h>
#include <KWindowSystem>
#include <QBoxLayout>
#include <QPainter>
#include <QPixmap>
#include <QRect>
#include <QSizePolicy>

#include <kiran-style/style-palette.h>

#include "applet.h"
#include "lib/common/utility.h"
#include "lib/common/window-info-helper.h"

namespace Kiran
{
namespace Taskbar
{
Applet::Applet(IAppletImport *import)
    : m_import(import)
{
    //最大化
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_appButtonContainer = new AppButtonContainer(m_import, this);

    QGridLayout *gridLayout = new QGridLayout(this);
    gridLayout->setMargin(0);
    gridLayout->setSpacing(0);
    gridLayout->addWidget(m_appButtonContainer);

    connect(KWindowSystem::self(), &KWindowSystem::windowAdded, this, &Applet::addWindow);
    connect(KWindowSystem::self(), &KWindowSystem::windowRemoved, this, &Applet::removeWindow);
    connect(KWindowSystem::self(), &KWindowSystem::activeWindowChanged, this, &Applet::changedActiveWindow);

    //    connect(KWindowSystem::self(), SIGNAL(windowChanged(WId, NET::Properties, NET::Properties2)), this, SLOT(changedWindow(WId, NET::Properties, NET::Properties2)));

    //    connect(KWindowSystem::self(),
    //            static_cast<void (KWindowSystem::*)(WId, NET::Properties, NET::Properties2)>(&KWindowSystem::windowChanged),
    //            this,
    //            &AppButton::changedWindow);

    connect(KWindowSystem::self(),
            QOverload<WId, NET::Properties, NET::Properties2>::of(
                &KWindowSystem::windowChanged),
            m_appButtonContainer,
            &AppButtonContainer::windowChanged);
}

Applet::~Applet()
{
}

void Applet::addWindow(WId wid)
{
    //    if (KWindowSystem::isPlatformX11())
    if (!WindowInfoHelper::isSkipTaskbar(wid))
    {
        emit windowAdded(wid);
    }
}

void Applet::removeWindow(WId wid)
{
    if (!WindowInfoHelper::isSkipTaskbar(wid))
    {
        emit windowRemoved(wid);
    }
}

void Applet::changedActiveWindow(WId wid)
{
    if (!WindowInfoHelper::isSkipTaskbar(wid))
    {
        emit activeWindowChanged(wid);
    }
}

}  // namespace Taskbar

}  // namespace Kiran
