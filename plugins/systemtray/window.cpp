/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
 * kiran-shell is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan
 * PSL v2. You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 * NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE. See the
 * Mulan PSL v2 for more details.
 *
 * Author:     yangfeng <yangfeng@kylinsec.com.cn>
 */

#include <ks-i.h>
#include <qt5-log-i.h>
#include <QDragEnterEvent>
#include <QUuid>

#include "lib/common/utility.h"
#include "tray-item.h"
#include "tray-server.h"
#include "window.h"

#define SERVICE_NAME QLatin1String("org.kde.StatusNotifierWatcher")
#define WATCHER_PATH QLatin1String("/StatusNotifierWatcher")
#define FREEDESKTOP_PROPERTIES QLatin1String("org.freedesktop.DBus.Properties")
#define REGISTERED_ITEMS QLatin1String("RegisteredStatusNotifierItems")
#define REGISTERED_HOST QLatin1String("RegisterStatusNotifierHost")

#define LAYOUT_MARGIN 10

namespace Kiran
{
namespace Systemtray
{
Window::Window(IAppletImport *import, QWidget *parent)
    : KiranColorBlock(parent), m_import(import), m_windowPopup(nullptr), m_updateWindowPopupPosInProgress(false)
{
    setAcceptDrops(true);
    setRadius(0);

    auto size = m_import->getPanel()->getSize() / 40 * 32;

    auto direction = getLayoutDirection();
    m_layoutBase = new QBoxLayout(direction, this);
    m_layoutBase->setSpacing(0);

    m_indicatorWidget = new StyledButton(this);
    m_indicatorWidget->setChecked(true);  // 显示不一样的样式
    m_indicatorWidget->setFixedSize(size, size);
    m_indicatorWidget->hide();

    // 托盘弹出窗口按钮
    m_windowPopupButton = new StyledButton(this);
    m_windowPopupButton->setIcon(QIcon::fromTheme(KS_ICON_TRAY_BOX));

    m_windowPopupButton->setFixedSize(size, size);
    m_layoutBase->addWidget(m_windowPopupButton);

    m_layout = new QBoxLayout(direction);
    m_layout->setMargin(0);
    m_layout->setSpacing(0);
    m_layoutBase->addLayout(m_layout);

    // 托盘弹出窗口
    m_windowPopup = new WindowPopup(import, this);
    connect(m_windowPopup, &WindowPopup::hideTrayBox, this, &Window::hideTrayBox);
    connect(m_windowPopup, &WindowPopup::dropEnded, this, &Window::dropEnd);
    connect(m_windowPopup, &WindowPopup::updatePosition, this,
            &Window::startUpdateTrayBoxPos);
    connect(this, &Window::dropEnded, m_windowPopup, &WindowPopup::dropEnd);
    connect(m_windowPopupButton, &QPushButton::clicked, this,
            [this](bool checked)
            {
                if (m_windowPopup->isHidden())
                {
                    startUpdateTrayBoxPos();
                    m_windowPopup->show();
                    m_windowPopupButton->setEnabled(false);
                }
            });

    updateLayout();

    // 布局更新
    QObject *Object = dynamic_cast<QObject *>(m_import->getPanel());
    connect(Object, SIGNAL(panelProfileChanged()), this, SLOT(updateLayout()));

    // 托盘服务注册
    TrayServerInstance;

    // 托盘显示服务注册
    static int statusNotifierHostIndex = 1;
    m_statusNotifierHostName = QString("org.kde.StatusNotifierHost-%1-%2")
                                   .arg(QCoreApplication::applicationPid())
                                   .arg(statusNotifierHostIndex++);
    QDBusConnection::sessionBus().registerService(m_statusNotifierHostName);

    // 托盘服务监控
    m_statusNotifierWatcherInterface = new org::kde::StatusNotifierWatcher(
        SERVICE_NAME, WATCHER_PATH, QDBusConnection::sessionBus(), this);
    connect(m_statusNotifierWatcherInterface,
            &OrgKdeStatusNotifierWatcherInterface::StatusNotifierItemRegistered,
            this, &Window::statusNotifierItemRegister);
    connect(m_statusNotifierWatcherInterface,
            &OrgKdeStatusNotifierWatcherInterface::StatusNotifierItemUnregistered,
            this, &Window::statusNotifierItemUnregister);

    // 此步骤需要，虽然本程序不管理host，但为了确保当StatusNotifierWatcher服务由其他进程控制时，若需要管理host能识别到本程序
    m_statusNotifierWatcherInterface->RegisterStatusNotifierHost(
        m_statusNotifierHostName);

    getRegisteredItems();
}

Window::~Window()
{
    if (m_windowPopup)
    {
        delete m_windowPopup;
        m_windowPopup = nullptr;
    }

    QDBusConnection::sessionBus().unregisterService(m_statusNotifierHostName);
}

QBoxLayout::Direction Window::getLayoutDirection()
{
    int orientation = m_import->getPanel()->getOrientation();
    auto direction = (orientation == PanelOrientation::PANEL_ORIENTATION_BOTTOM ||
                      orientation == PanelOrientation::PANEL_ORIENTATION_TOP)
                         ? QBoxLayout::Direction::LeftToRight
                         : QBoxLayout::Direction::TopToBottom;
    return direction;
}

Qt::AlignmentFlag Window::getLayoutAlignment()
{
    int orientation = m_import->getPanel()->getOrientation();
    Qt::AlignmentFlag alignment =
        (orientation == PanelOrientation::PANEL_ORIENTATION_BOTTOM ||
         orientation == PanelOrientation::PANEL_ORIENTATION_TOP)
            ? Qt::AlignLeft
            : Qt::AlignTop;

    return alignment;
}

void Window::getRegisteredItems()
{
    QDBusMessage msg = QDBusMessage::createMethodCall(
        m_statusNotifierWatcherInterface->service(),
        m_statusNotifierWatcherInterface->path(), FREEDESKTOP_PROPERTIES,
        QLatin1String("Get"));
    msg << m_statusNotifierWatcherInterface->interface() << REGISTERED_ITEMS;
    QDBusPendingCall call =
        m_statusNotifierWatcherInterface->connection().asyncCall(msg);
    call.waitForFinished();
    if (call.isValid())
    {
        for (QVariant value : call.reply().arguments())
        {
            QDBusVariant dbusVariant = qvariant_cast<QDBusVariant>(value);
            QStringList items = dbusVariant.variant().toStringList();
            for (QString item : items)
            {
                statusNotifierItemRegister(item);
            }
        }
    }
}

void Window::statusNotifierItemRegister(const QString &serviceAndPath)
{
    if (m_windowPopup->isWindowPopupItem(serviceAndPath))
    {
        m_windowPopup->AddItem(serviceAndPath);
        return;
    }
    if (m_services.contains(serviceAndPath))
    {
        return;
    }

    auto item = itemAdd(serviceAndPath);
    if (item)
    {
        m_items.append(item);
        updateItemLayout();
    }
}

void Window::statusNotifierItemUnregister(const QString &serviceAndPath)
{
    itemRemove(serviceAndPath);
}

TrayItem *Window::itemAdd(QString serviceAndPath)
{
    int index = serviceAndPath.indexOf('/');
    QString service = serviceAndPath.left(index);
    QString path = serviceAndPath.mid(index);
    auto item = new TrayItem(service, path, this);
    auto size = m_import->getPanel()->getSize() / 40 * 32;
    item->setFixedSize(size, size);
    connect(item, &TrayItem::startDrag, this, [this](TrayItem *dragItem)
            {
                m_windowPopup->show();
                m_windowPopupButton->setEnabled(false);
                // 为什么不隐藏拖拽的item ？
                // 存在一种情况，当图标被拖出去，没有放到正确的位置去
                // 这种情况下，无法检测发生了什么
                // 只能保持显示，等拖拽成功了之后，再隐藏删除
                // 另：直接将图标拖到显示区域外，不会经过 dragLeaveEvent

                // m_items.removeAll(dragItem);
                // dragItem->hide();

                updateItemLayout();
            });
    m_services.insert(serviceAndPath, item);

    return item;
}

void Window::itemRemove(const QString &serviceAndPath)
{
    auto item = m_services.value(serviceAndPath);
    if (item)
    {
        m_services.remove(serviceAndPath);
        m_items.removeAll(item);
        item->deleteLater();
    }
    updateItemLayout();
}

void Window::startUpdateTrayBoxPos()
{
    if (!m_updateWindowPopupPosInProgress)
    {
        // 需要延迟处理，window大小变化后，只能获得之前的位置坐标
        // 避免短时间内多次调用
        m_updateWindowPopupPosInProgress = true;
        QTimer::singleShot(100, this, [this]()
                           {
                               updateTrayBoxPos();
                           });
    }
}

void Window::updateTrayBoxPos()
{
    QPoint pos = mapToGlobal(m_windowPopupButton->pos());
    if (QBoxLayout::Direction::LeftToRight == getLayoutDirection())
    {
        m_windowPopup->move(pos.x() - m_windowPopup->width() / 2 +
                                m_windowPopupButton->width() / 2,
                            pos.y());
    }
    else
    {
        m_windowPopup->move(pos.x(), pos.y() - m_windowPopup->height() / 2 +
                                         m_windowPopupButton->height() / 2);
    }

    m_updateWindowPopupPosInProgress = false;
}

void Window::hideTrayBox()
{
    // KLOG_INFO() << "Window::hideTrayBox";
    m_windowPopup->hide();
    m_windowPopupButton->setEnabled(true);
    m_windowPopupButton->setChecked(false);
}

void Window::dropEnd(QString serviceAndPath)
{
    itemRemove(serviceAndPath);
}

int Window::getInsertedIndex(const QPoint &pos)
{
    // 将区域等分划分
    // 计算鼠标位置，确定插入位置
    int itemCount = m_items.size();
    if (0 == itemCount)
    {
        return 0;
    }

    int cellWidth = width() / itemCount;
    int col = pos.x() / cellWidth;

    return col;
}

void Window::updateItemLayout()
{
    Utility::clearLayout(m_layout);

    // 大小增加一个size，用于托盘弹出窗口按钮
    auto itemWidth = m_import->getPanel()->getSize() / 40 * 32;
    auto height = m_import->getPanel()->getSize();
    if (QBoxLayout::Direction::LeftToRight == getLayoutDirection())
    {
        setFixedSize(m_items.size() * itemWidth + itemWidth + LAYOUT_MARGIN * 2,
                     height);
    }
    else
    {
        setFixedSize(height,
                     m_items.size() * itemWidth + itemWidth + LAYOUT_MARGIN * 2);
    }
    for (auto item : m_items)
    {
        m_layout->addWidget(item);
    }

    startUpdateTrayBoxPos();
}

QList<TrayItem *> Window::getTrayItems()
{
    return m_services.values();
}

WindowPopup *Window::getTrayBox()
{
    return m_windowPopup;
}

void Window::dragEnterEvent(QDragEnterEvent *event)
{
    KLOG_INFO() << "Window::dragEnterEvent";

    m_currentDropIndex = 0;
    if (!event->mimeData()->data("serviceAndPath").isEmpty())
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void Window::dragMoveEvent(QDragMoveEvent *event)
{
    QPoint pos = event->pos();
    int index = getInsertedIndex(pos);

    if (index != m_items.indexOf(m_indicatorWidget))
    {
        m_items.removeAll(m_indicatorWidget);

        // 直接使用insert，效果一样，但是Qt有警告
        if (index >= m_items.size())
        {
            m_items.append(m_indicatorWidget);
        }
        else
        {
            m_items.insert(index, m_indicatorWidget);
        }
        m_indicatorWidget->show();
    }

    m_currentDropIndex = index;
    updateItemLayout();

    event->accept();
}

void Window::dragLeaveEvent(QDragLeaveEvent *event)
{
    m_currentDropIndex = 0;
    m_items.removeAll(m_indicatorWidget);
    m_indicatorWidget->hide();
    updateItemLayout();

    event->accept();
}

void Window::dropEvent(QDropEvent *event)
{
    QByteArray serviceAndPath = event->mimeData()->data("serviceAndPath");
    if (serviceAndPath.isEmpty())
    {
        QWidget::dropEvent(event);
        return;
    }

    // 不存在，则是其他区域拖过来的
    if (!m_services.contains(serviceAndPath))
    {
        TrayItem *item = itemAdd(serviceAndPath);
        m_items.insert(m_currentDropIndex, item);
        emit dropEnded(serviceAndPath);
    }
    else
    {
        // 存在，调整位置
        TrayItem *item = m_services.value(serviceAndPath);
        int oldIndex = m_items.indexOf(item);
        m_items.move(oldIndex, m_currentDropIndex);
    }

    m_items.removeAll(m_indicatorWidget);
    m_indicatorWidget->hide();

    updateItemLayout();

    event->accept();
}

void Window::updateLayout()
{
    // 横竖摆放
    auto direction = getLayoutDirection();
    m_layoutBase->setDirection(direction);
    m_layout->setDirection(direction);

    // 子控件对齐方式：左右、上下
    Qt::AlignmentFlag alignment = getLayoutAlignment();
    m_layout->setAlignment(alignment);
    m_layoutBase->setAlignment(alignment);
    updateItemLayout();

    if (QBoxLayout::Direction::LeftToRight == direction)
    {
        m_layoutBase->setContentsMargins(LAYOUT_MARGIN, 4, LAYOUT_MARGIN, 4);
    }
    else
    {
        m_layoutBase->setContentsMargins(4, LAYOUT_MARGIN, 4, LAYOUT_MARGIN);
    }
}

}  // namespace Systemtray
}  // namespace Kiran
