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

#include <kiran-integration/theme/palette.h>
#include <qt5-log-i.h>
#include <KWindowSystem>
#include <QCursor>
#include <QGSettings>
#include <QKeyEvent>
#include <QPainter>
#include <QScreen>

#include "desktop-helper.h"
#include "ks-i.h"
#include "lib/common/window-manager.h"
#include "ui_window.h"
#include "window.h"
#include "workspace-overview.h"
#include "workspace-thumbnail.h"

namespace Kiran
{
namespace Workspace
{
Window::Window(QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint),
      m_ui(new Ui::Window)
{
    m_ui->setupUi(this);
    setAttribute(Qt::WA_TranslucentBackground);

    m_ui->m_listWidgetThumbnail->setSpacing(10);
    QPalette palette = m_ui->m_listWidgetThumbnail->palette();
    palette.setBrush(QPalette::Base, Qt::NoBrush);    // 基本背景透明
    palette.setBrush(QPalette::Window, Qt::NoBrush);  // 窗口背景透明
    m_ui->m_listWidgetThumbnail->setPalette(palette);

    init();
}

Window::~Window()
{
    delete m_ui;
}

void Window::paintEvent(QPaintEvent *event)
{
    auto gsettings = QSharedPointer<QGSettings>(new QGSettings(KIRAN_APPEARANCE_SCHEMA_ID));
    if (gsettings.isNull())
    {
        KLOG_ERROR() << KIRAN_APPEARANCE_SCHEMA_ID << "QGSettings schema create failed";
        return;
    }
    auto desktopBackgroundValue = gsettings->get(DESKTOP_BACKGROUND_SCHEMA_KEY);
    if (desktopBackgroundValue.isNull() || !desktopBackgroundValue.isValid())
    {
        KLOG_ERROR() << KIRAN_APPEARANCE_SCHEMA_ID << DESKTOP_BACKGROUND_SCHEMA_KEY << "QGSettings key get failed";
        return;
    }
    auto desktopBackground = desktopBackgroundValue.toString();

    QPainter painter(this);

    QPixmap background(desktopBackground);

    painter.drawPixmap(rect(), background);

    auto kiranPalette = Kiran::Theme::Palette::getDefault();

    // 添加一层
    auto bg = kiranPalette->getBaseColors().baseBackground;
    bg.setAlpha(50);
    painter.setPen(Qt::NoPen);
    painter.setBrush(bg);
    painter.drawRect(rect());

    m_ui->m_widgetThumbnail->setAttribute(Qt::WA_TranslucentBackground);
    bg.setAlpha(120);
    painter.setBrush(bg);
    painter.drawRect(m_ui->m_widgetThumbnail->geometry());

    QWidget::paintEvent(event);
}

void Window::showEvent(QShowEvent *event)
{
    //任务栏不显示
    KWindowSystem::setState(winId(), NET::SkipTaskbar | NET::SkipPager | NET::SkipSwitcher);

#if 0  // 调试用,将显示画面缩小
    // 鼠标所在的屏幕
    QPoint cursorPos = QCursor::pos();
    QScreen *screen = QApplication::screenAt(cursorPos);
    if (screen)
    {
        QRect screenGeometry = screen->geometry();
        int width = screenGeometry.width();
        int height = screenGeometry.height();
        setFixedSize(width / 3 * 2, height / 3 * 2);
    }
#else
    showFullScreen();
#endif
}

void Window::mouseReleaseEvent(QMouseEvent *event)
{
    emit windowDeactivated();
}

void Window::keyPressEvent(QKeyEvent *event)
{
    if (Qt::Key_Escape == event->key())
    {
        emit windowDeactivated();
    }

    QWidget::keyPressEvent(event);
}

bool Window::eventFilter(QObject *object, QEvent *event)
{
    if (QEvent::WindowDeactivate == event->type())
    {
        emit windowDeactivated();
    }

    return QWidget::eventFilter(object, event);
}

void Window::init()
{
    int numOfDesk = DesktopHelper::numberOfDesktops();
    changeNumberOfDesktops(numOfDesk);

    m_desktopHelper = new DesktopHelper(this);
    connect(m_desktopHelper, &DesktopHelper::currentDesktopChanged, this, &Window::changeCurrentDesktop);
    connect(m_desktopHelper, &DesktopHelper::numberOfDesktopsChanged, this, &Window::changeNumberOfDesktops);

    changeCurrentDesktop(DesktopHelper::currentDesktop());

    //事件过滤器
    installEventFilter(this);
}

void Window::createDesktop()
{
    DesktopHelper::createDesktop();
    // KWindowSystem 没有提供工作区删除或创建的信号
    // KWindowSystem::numberOfDesktopsChanged 是异步信号,所以这里不需要添加界面元素,等信号来了再添加
    // addWorkspace(m_desktopHelper->numberOfDesktops());
}

void Window::removeDesktop(int desktop)
{
    DesktopHelper::removeDesktop(desktop);
}

void Window::addWorkspace(int desktop)
{
    auto thumbnail = new WorkspaceThumbnail(desktop, this);
    auto overview = new WorkspaceOverview(desktop, this);

    m_workspaces[desktop] = qMakePair(thumbnail, overview);

    QListWidgetItem *item = new QListWidgetItem(m_ui->m_listWidgetThumbnail);
    item->setSizeHint(QSize(1, 120));

    m_ui->m_listWidgetThumbnail->addItem(item);
    m_ui->m_listWidgetThumbnail->setItemWidget(item, thumbnail);

    connect(thumbnail, &WorkspaceThumbnail::removeDesktop, this, &Window::removeDesktop);
}

void Window::clearWorkspace()
{
    auto it = m_workspaces.begin();
    while (it != m_workspaces.end())
    {
        auto thumbnail = it.value().first;
        auto overview = it.value().second;

        delete thumbnail;
        delete overview;

        it = m_workspaces.erase(it);
    }
    m_ui->m_listWidgetThumbnail->clear();
}

void Window::changeCurrentDesktop(int desktop)
{
    if (!m_workspaces.contains(desktop))
    {
        return;
    }

    auto workspace = m_workspaces[desktop];
    auto thumbnail = workspace.first;
    auto overview = workspace.second;

    thumbnail->updatePreviewer();

    QLayoutItem *item;
    while ((item = m_ui->m_layoutOverview->takeAt(0)) != nullptr)
    {
        if (item->widget())
        {
            item->widget()->hide();
        }

        delete item;
    }

    m_ui->m_layoutOverview->addWidget(overview);
    overview->show();
}

void Window::changeNumberOfDesktops(int numOfDesk)
{
    // 这里由于不知道是新增还是减少导致的窗口数变化,所以先清空所有的界面元素,再重新创建
    // 接收到这个信号的原因:1.工作区缩略图点关闭  2.外部(如其他面板,其他程序)执行减少工作区操作
    clearWorkspace();

    for (int i = 1; i <= numOfDesk; i++)
    {
        addWorkspace(i);
    }

    changeCurrentDesktop(DesktopHelper::currentDesktop());
}

void Window::on_m_btnAddWorkspace_clicked()
{
    createDesktop();
}

void Window::on_m_listWidgetThumbnail_itemDoubleClicked(QListWidgetItem *item)
{
    DesktopHelper::setCurrentDesktop(m_ui->m_listWidgetThumbnail->row(item) + 1);
}

void Window::on_m_listWidgetThumbnail_currentRowChanged(int currentRow)
{
    changeCurrentDesktop(currentRow + 1);
}

}  //     namespace Workspace
}  //     namespace Kiran
