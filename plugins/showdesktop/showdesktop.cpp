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
#include <QBoxLayout>
#include <QCoreApplication>
#include <QTranslator>

#include "ks-config.h"
#include "ks-i.h"
#include "lib/common/window-manager.h"
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
                WindowManagerInstance.setShowingDesktop(!WindowManagerInstance.isShowingDesktop());
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

// 根据 panel 尺寸和方向更新显示桌面按钮布局
// 横向panel：按钮高度与panel一致，宽度为1/4，上下保留4px边距
// 纵向panel：按钮宽度与panel一致，高度为1/4，左右保留4px边距
void Showdesktop::updateLayout()
{
    KLOG_WARNING() << "updateLayout";

    int orientation = m_import->getPanel()->getOrientation();
    auto size = m_import->getPanel()->getSize();
    auto *boxLayout = static_cast<QBoxLayout *>(layout());

    if (PanelOrientation::PANEL_ORIENTATION_BOTTOM == orientation ||
        PanelOrientation::PANEL_ORIENTATION_TOP == orientation)
    {
        setFixedSize(size / 4, size);
        boxLayout->setContentsMargins(0, 4, 0, 4);
        m_button->setFixedSize(size / 4, size - 8);
    }
    else
    {
        setFixedSize(size, size / 4);
        boxLayout->setContentsMargins(4, 0, 4, 0);
        m_button->setFixedSize(size - 8, size / 4);
    }
}

}  // namespace Kiran
