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

#include <qt5-log-i.h>
#include <QDragEnterEvent>
#include <QGSettings>

#include "ks-i.h"
#include "lib/common/utility.h"
#include "statusnotifieriteminterface.h"
#include "tray-extended.h"

namespace Kiran
{
namespace Systemtray
{
TrayExtended::TrayExtended(IAppletImport *import, QWidget *parent)
    : QDialog(parent, Qt::WindowFlags() | Qt::FramelessWindowHint),
      m_import(import),
      m_currentDropIndex(0)
{
    m_layout = new QGridLayout(this);
    m_layout->setMargin(0);
    m_layout->setSpacing(0);

    m_indicatorWidget = new StyledButton(this);
    m_indicatorWidget->setChecked(true);  // 显示不一样的样式
    auto size = m_import->getPanel()->getSize();
    m_indicatorWidget->setFixedSize(size, size);
    m_indicatorWidget->hide();

    setMinimumSize(size, size);

    setAcceptDrops(true);

    updateItemLayout();
    // 事件过滤器
    installEventFilter(this);
}

TrayExtended::~TrayExtended()
{
}

QList<TrayItem *> TrayExtended::getTrayItems()
{
    return m_services.values();
}

void TrayExtended::dropEnd(QString serviceAndPath)
{
    itemRemove(serviceAndPath);

    removeFoldingItem(serviceAndPath);
}

void TrayExtended::AddItem(QString serviceAndPath)
{
    if (m_services.contains(serviceAndPath))
    {
        // 注册时进入，如果是服务重启，也会导致再次注册
        return;
    }

    auto *item = itemAdd(serviceAndPath);
    m_items.append(item);
    updateItemLayout();
}

bool TrayExtended::eventFilter(QObject *object, QEvent *event)
{
    // window was deactivated
    if (QEvent::WindowDeactivate == event->type())
    {
        emit hideTrayExtended();
    }
    return QWidget::eventFilter(object, event);
}

void TrayExtended::dragEnterEvent(QDragEnterEvent *event)
{
    m_currentDropIndex = 0;
    if (!event->mimeData()->data("serviceAndPath").isEmpty())
    {
        QPoint pos = event->pos();
        updateDragPos(pos);

        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void TrayExtended::dragMoveEvent(QDragMoveEvent *event)
{
    QPoint pos = event->pos();
    updateDragPos(pos);

    event->accept();
}

void TrayExtended::dragLeaveEvent(QDragLeaveEvent *event)
{
    m_currentDropIndex = 0;
    m_items.removeAll(m_indicatorWidget);
    m_indicatorWidget->hide();

    updateItemLayout();

    event->accept();
}

void TrayExtended::dropEvent(QDropEvent *event)
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

        addFoldingItem(serviceAndPath);
    }
    else  // 存在，调整位置
    {
        TrayItem *item = m_services.value(serviceAndPath);
        m_items.removeAll(item);
        m_items.insert(m_currentDropIndex, item);
    }

    m_items.removeAll(m_indicatorWidget);
    m_indicatorWidget->hide();

    updateItemLayout();

    event->accept();
}

void TrayExtended::hideEvent(QHideEvent *event)
{
    m_items.removeAll(m_indicatorWidget);
    m_indicatorWidget->hide();

    updateItemLayout();
}

int TrayExtended::getInsertIndex(const QPoint &pos)
{
    // 将区域等分划分
    // 计算鼠标位置，确定插入位置
    int rowCount = 1;
    int colCount = 1;
    calculateRowCol(m_items.size(), rowCount, colCount);

    int cellWidth = width() / colCount;
    int cellHeight = height() / rowCount;
    int row = pos.y() / cellHeight;
    int col = pos.x() / cellWidth;

    int index = row * colCount + col;

    return index;
}

void TrayExtended::updateItemLayout()
{
    Utility::clearLayout(m_layout);

    int rowCount = 1;
    int colCount = 1;
    calculateRowCol(m_items.size(), rowCount, colCount);

    auto size = m_import->getPanel()->getSize();
    setFixedSize(colCount * size, rowCount * size);

    for (int row = 0; row < rowCount; row++)
    {
        for (int col = 0; col < colCount; col++)
        {
            int index = row * rowCount + col;
            if (index >= m_items.size())
            {
                break;
            }
            m_layout->addWidget(m_items.at(index), row, col);
        }
    }

    emit updatePosition();
}

void TrayExtended::calculateRowCol(const int &totalSize, int &row, int &col)
{
    // 重新排序显示
    // 没有子项时，布局为 1x1
    // 只有1个子项时，布局为 1x2
    // 超过1个子项时，为 x^2
    row = 1;
    col = 1;
    int rowAndCol = std::ceil(std::sqrt(totalSize));
    if (2 == totalSize)
    {
        col = 2;
    }
    else if (rowAndCol >= 1)
    {
        row = rowAndCol;
        col = rowAndCol;
    }
}

void TrayExtended::updateDragPos(const QPoint &pos)
{
    if (1 == m_items.size())
    {
        m_items.removeAll(m_indicatorWidget);
        m_items.append(m_indicatorWidget);
    }
    int index = getInsertIndex(pos);

    if (index != m_items.indexOf(m_indicatorWidget))
    {
        m_items.removeAll(m_indicatorWidget);

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
}

TrayItem *TrayExtended::itemAdd(QString serviceAndPath)
{
    int index = serviceAndPath.indexOf('/');
    QString service = serviceAndPath.left(index);
    QString path = serviceAndPath.mid(index);
    TrayItem *item = new TrayItem(service, path, this);
    auto size = m_import->getPanel()->getSize();
    item->setFixedSize(size, size);

    connect(item, &TrayItem::startDrag, this, [this](TrayItem *dragItem)
            {
                // m_items.removeAll(dragItem);
                // dragItem->hide();
                updateItemLayout();
            });
    m_services.insert(serviceAndPath, item);

    return item;
}

void TrayExtended::itemRemove(const QString &serviceAndPath)
{
    auto item = m_services.value(serviceAndPath);
    if (item)
    {
        m_items.removeAll(item);
        m_services.remove(serviceAndPath);
        item->deleteLater();
    }

    updateItemLayout();
}

void TrayExtended::addFoldingItem(const QString &serviceAndPath)
{
    QString itemId = getStatusNotifierItemId(serviceAndPath);
    if (itemId.isEmpty())
    {
        return;
    }

    auto gsettings = QSharedPointer<QGSettings>(new QGSettings(SYSTEMTRAY_SCHEMA_ID));
    QVariantList foldingApps = gsettings->get(SYSTEMTRAY_SCHEMA_KEY_FOLDING_APPS).toList();
    if (!foldingApps.contains(itemId))
    {
        foldingApps.append(itemId);
        gsettings->set(SYSTEMTRAY_SCHEMA_KEY_FOLDING_APPS, foldingApps);
    }
}

void TrayExtended::removeFoldingItem(const QString &serviceAndPath)
{
    QString itemId = getStatusNotifierItemId(serviceAndPath);
    if (itemId.isEmpty())
    {
        return;
    }

    auto gsettings = QSharedPointer<QGSettings>(new QGSettings(SYSTEMTRAY_SCHEMA_ID));
    QVariantList foldingApps = gsettings->get(SYSTEMTRAY_SCHEMA_KEY_FOLDING_APPS).toList();
    if (foldingApps.contains(itemId))
    {
        foldingApps.removeAll(itemId);
        gsettings->set(SYSTEMTRAY_SCHEMA_KEY_FOLDING_APPS, foldingApps);
    }
}

bool TrayExtended::isWindowPopupItem(const QString &serviceAndPath)
{
    QString itemId = getStatusNotifierItemId(serviceAndPath);
    if (itemId.isEmpty())
    {
        return false;
    }

    auto gsettings = QSharedPointer<QGSettings>(new QGSettings(SYSTEMTRAY_SCHEMA_ID));
    QVariantList foldingApps = gsettings->get(SYSTEMTRAY_SCHEMA_KEY_FOLDING_APPS).toList();
    return foldingApps.contains(itemId);
}

QString TrayExtended::getStatusNotifierItemId(const QString &serviceAndPath)
{
    int index = serviceAndPath.indexOf('/');
    QString service = serviceAndPath.left(index);
    QString path = serviceAndPath.mid(index);
    OrgKdeStatusNotifierItem statusNotifierItem(service, path, QDBusConnection::sessionBus());
    if (statusNotifierItem.isValid())
    {
        return statusNotifierItem.id();
    }
    return "";
}

}  // namespace Systemtray
}  // namespace Kiran
