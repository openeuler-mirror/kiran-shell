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
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#include <qt5-log-i.h>
#include <KWindowSystem/KWindowSystem>
#include <QBoxLayout>
#include <QCoreApplication>
#include <QTranslator>

#include "ks-config.h"
#include "ks-i.h"
#include "showdesktop.h"

namespace Kiran
{
Showdesktop::Showdesktop(IAppletImport *import)
    : m_import(import),
      m_button(nullptr)
{
    static QTranslator translator;
    if (!translator.load(QLocale(), "showdesktop", ".", KS_INSTALL_TRANSLATIONDIR, ".qm"))
    {
        KLOG_WARNING() << "Load translator failed!";
    }
    else
    {
        QCoreApplication::installTranslator(&translator);
    }

    setRadius(0);

    m_button = new StyledButton(this);

    connect(m_button, &QAbstractButton::clicked, this, [=]()
            {
                KWindowSystem::setShowingDesktop(!KWindowSystem::showingDesktop());
            });

    auto *layout = new QBoxLayout(QBoxLayout::Direction::LeftToRight, this);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(m_button);

    auto *panelObject = dynamic_cast<QObject *>(m_import->getPanel());
    connect(panelObject, SIGNAL(panelProfileChanged()), this, SLOT(updateLayout()));

    m_button->setToolTip(tr("Show desktop"));
    m_button->setCheckable(false);

    updateLayout();
}

void Showdesktop::updateLayout()
{
    KLOG_WARNING() << "updateLayout";

    int orientation = m_import->getPanel()->getOrientation();
    auto size = m_import->getPanel()->getSize();

    if (PanelOrientation::PANEL_ORIENTATION_BOTTOM == orientation ||
        PanelOrientation::PANEL_ORIENTATION_TOP == orientation)
    {
        setFixedSize(size / 4, size);
    }
    else
    {
        setFixedSize(size, size / 4);
    }
}

}  // namespace Kiran
