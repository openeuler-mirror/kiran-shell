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
    : m_import(import),
      m_appButtonContainer(nullptr)
{
    //最大化
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QObject *Object = dynamic_cast<QObject *>(m_import->getPanel());
    bool ret = connect(Object, SIGNAL(panelProfileChanged()), this, SLOT(updateLayout()));

    //横竖摆放
    auto direction = getLayoutDirection();
    m_layout = new QBoxLayout(direction, this);
    setLayout(m_layout);
    m_layout->setMargin(0);
    m_layout->setSpacing(3);

    //子控件排列方式：左右、上下
    Qt::AlignmentFlag alignment = getLayoutAlignment();
    m_layout->setAlignment(alignment);

    m_appButtonContainer = new AppButtonContainer(m_import, this);
    connect(this, &Applet::windowAdded, m_appButtonContainer, &AppButtonContainer::addWindow);
    connect(this, &Applet::windowRemoved, m_appButtonContainer, &AppButtonContainer::removedWindow);
    connect(this, &Applet::activeWindowChanged, m_appButtonContainer, &AppButtonContainer::changedActiveWindow);
    connect(m_appButtonContainer, &AppButtonContainer::appRefreshed, this, &Applet::updateAppShow);

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

void Applet::addWindow(WId id)
{
    //    if (KWindowSystem::isPlatformX11())
    if (!WindowInfoHelper::isSkipTaskbar(id))
    {
        emit windowAdded(id);
    }
}

void Applet::removeWindow(WId id)
{
    if (!WindowInfoHelper::isSkipTaskbar(id))
    {
        emit windowRemoved(id);
    }
}

void Applet::changedActiveWindow(WId id)
{
    if (!WindowInfoHelper::isSkipTaskbar(id))
    {
        emit activeWindowChanged(id);
    }
}

void Applet::updateAppShow()
{
    Utility::clearLayout(m_layout);

    auto appButtons = m_appButtonContainer->getAppButtons();
    for (auto button : appButtons)
    {
        m_layout->addWidget(button);
    }
}

void Applet::updateLayout()
{
    // KLOG_INFO() << "Applet::UpdateLayout";
    //横竖摆放
    auto direction = getLayoutDirection();
    m_layout->setDirection(direction);

    //子控件排列方式：左右、上下
    Qt::AlignmentFlag alignment = getLayoutAlignment();
    m_layout->setAlignment(alignment);
}

QBoxLayout::Direction Applet::getLayoutDirection()
{
    int orientation = m_import->getPanel()->getOrientation();
    auto direction = (orientation == PanelOrientation::PANEL_ORIENTATION_BOTTOM ||
                      orientation == PanelOrientation::PANEL_ORIENTATION_TOP)
                         ? QBoxLayout::Direction::LeftToRight
                         : QBoxLayout::Direction::TopToBottom;
    return direction;
}

Qt::AlignmentFlag Applet::getLayoutAlignment()
{
    int orientation = m_import->getPanel()->getOrientation();
    Qt::AlignmentFlag alignment = (orientation == PanelOrientation::PANEL_ORIENTATION_BOTTOM ||
                                   orientation == PanelOrientation::PANEL_ORIENTATION_TOP)
                                      ? Qt::AlignLeft
                                      : Qt::AlignTop;

    return alignment;
}

}  // namespace Taskbar

}  // namespace Kiran
