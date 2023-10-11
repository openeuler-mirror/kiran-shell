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

#include "plugins/menu/window.h"
#include <qt5-log-i.h>
#include <KService/KServiceGroup>
#include <QButtonGroup>
#include <QStackedWidget>
#include "plugins/menu/apps-overview.h"
#include "plugins/menu/recent-files-overview.h"
#include "plugins/menu/ui_window.h"

namespace Kiran
{
namespace Menu
{
Window::Window(QWidget *parent) : QWidget(parent, Qt::FramelessWindowHint),
                                  m_ui(new Ui::Window)
{
    this->m_ui->setupUi(this);

    this->init();
}

Window::~Window()
{
    delete this->m_ui;
}
// void recursiveService(KServiceGroup *serviceGroup)
// {
//     KServiceGroup::List list = serviceGroup->entries();

//     KLOG_DEBUG() << "entry number: " << list.size() << "group: " << serviceGroup->name();

//     // Iterate over all entries in the group
//     for (KServiceGroup::List::ConstIterator it = list.begin(); it != list.end(); it++)
//     {
//         KSycocaEntry *p = (*it).data();
//         KLOG_DEBUG() << "type: " << p->sycocaType();

//         if (p->isType(KST_KService))
//         {
//             KService *s = static_cast<KService *>(p);
//             KLOG_DEBUG() << "MenuID:   " << s->menuId() << "workingDirectory: " << s->path();
//         }
//         else if (p->isType(KST_KServiceGroup))
//         {
//             KServiceGroup *g = static_cast<KServiceGroup *>(p);
//             recursiveService(g);
//         }
//     }
// }

void Window::init()
{
    this->m_overviewSelections = new QButtonGroup(this);
    this->m_overviewSelections->addButton(this->m_ui->m_appsOverview, 0);
    this->m_overviewSelections->addButton(this->m_ui->m_recentFilesOverview, 1);
    // 移除qt designer默认创建的widget
    this->clear(this->m_ui->m_overviewStack);
    this->m_ui->m_overviewStack->addWidget(new AppsOverview());
    this->m_ui->m_overviewStack->addWidget(new RecentFilesOverview());

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    connect(this->m_overviewSelections, SIGNAL(idClicked(int)), this->m_ui->m_overviewStack, SLOT(setCurrentIndex(int)));
#else
    connect(this->m_overviewSelections, SIGNAL(buttonClicked(int)), this->m_ui->m_overviewStack, SLOT(setCurrentIndex(int)));
#endif

    // KServiceGroup::Ptr group = KServiceGroup::root();
    // recursiveService(group.data());
    // if (!group || !group->isValid()) return;

    // KServiceGroup::List list = group->entries();

    // KLOG_DEBUG() << "entry number: " << list.size();

    // // Iterate over all entries in the group
    // for (KServiceGroup::List::ConstIterator it = list.begin(); it != list.end(); it++)
    // {
    //     KSycocaEntry *p = (*it).data();
    //     KLOG_DEBUG() << "type: " << p->sycocaType();

    //     if (p->isType(KST_KService))
    //     {
    //         KService *s = static_cast<KService *>(p);
    //         KLOG_DEBUG() << "Name:   " << s->name();
    //     }
    // }
}

void Window::clear(QStackedWidget *stackedWidget)
{
    while (stackedWidget->currentWidget() != nullptr)
    {
        auto currentWidget = stackedWidget->currentWidget();
        stackedWidget->removeWidget(currentWidget);
        delete currentWidget;
    }
}

}  // namespace  Menu

}  // namespace Kiran
