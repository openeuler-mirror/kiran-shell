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
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#include <ks-i.h>
#include <qt5-log-i.h>
#include <KWindowSystem/KWindowSystem>
#include <QCoreApplication>
#include <QTranslator>

#include "ks-config.h"
#include "showdesktop.h"

namespace Kiran
{
Showdesktop::Showdesktop(IAppletImport *import)
    : m_import(import)
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

    connect(this, &QPushButton::clicked, this, [=]()
            { KWindowSystem::setShowingDesktop(!KWindowSystem::showingDesktop()); });

    QObject *Object = dynamic_cast<QObject *>(m_import->getPanel());
    bool ret = connect(Object, SIGNAL(panelProfileChanged()), this, SLOT(updateLayout()));

    updateLayout();

    setToolTip(tr("Show desktop"));
}

void Showdesktop::updateLayout()
{
    int orientation = m_import->getPanel()->getOrientation();

    if (orientation == PanelOrientation::PANEL_ORIENTATION_BOTTOM ||
        orientation == PanelOrientation::PANEL_ORIENTATION_TOP)
    {
        auto size = m_import->getPanel()->getSize();
        setFixedSize(size / 2, size);
    }
    else
    {
        auto size = m_import->getPanel()->getSize();
        setFixedSize(size, size / 2);
    }
}

}  // namespace Kiran
