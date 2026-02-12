/**
 * Copyright (c) 2026 ~ 2027 KylinSec Co., Ltd.
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

#include <QAbstractItemView>
#include <QApplication>
#include <QGridLayout>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMap>
#include <QModelIndex>
#include <QPointer>
#include <QRect>
#include <QTimer>
#include <QTreeView>
#include <algorithm>
#include <cmath>
#include <limits>

#include "apps-overview.h"
#include "key-navigation.h"
#include "recent-files-overview.h"
#include "tree-view.h"
#include "ui_window.h"
#include "window.h"

namespace Kiran
{
namespace Menu
{
KeyNavigation::KeyNavigation(Window *window, Ui::Window *ui)
    : QObject(window),
      m_window(window),
      m_ui(ui)
{
}

bool KeyNavigation::handleKeyPress(QKeyEvent *event)
{
    QWidget *focused = QApplication::focusWidget();
    if (!focused)
    {
        return false;
    }
    NavRegion region = regionOf(focused);

    // 焦点在搜索框时，左右键用于移动输入光标，不参与跨区域导航
    if (region == NavRegion::OverviewStack && event->key() != Qt::Key_Up && event->key() != Qt::Key_Down)
    {
        QWidget *overview = m_ui->widgetOverviewStack->currentWidget();
        if (overview)
        {
            QLineEdit *search = overview->findChild<QLineEdit *>(QStringLiteral("edit_search"));
            if (search && focused == search)
            {
                return false;  // 左/右键交给 QLineEdit 处理光标
            }
        }
    }

    // OverviewStack 内树形视图（应用列表/文件树）的特殊处理：
    // 上键到顶时切换到搜索框；下键到底时阻止默认行为
    QAbstractItemView *treeView = overviewTreeView();
    if (region == NavRegion::OverviewStack && treeView && (focused == treeView || focused == treeView->viewport()))
    {
        if (event->key() == Qt::Key_Up)
        {
            // 已在第一行，上键切换到搜索框
            QTreeView *tv = qobject_cast<QTreeView *>(treeView);
            QModelIndex cur = treeView->currentIndex();
            QModelIndex above = tv ? tv->indexAbove(cur) : QModelIndex();
            if (!above.isValid())
            {
                QLineEdit *search = m_ui->widgetOverviewStack->currentWidget()->findChild<QLineEdit *>(QStringLiteral("edit_search"));
                if (search)
                {
                    search->setFocus();
                    return true;
                }
            }
            return false;
        }
        if (event->key() == Qt::Key_Down)
        {
            // 已在最后一行，下键不交给 tree 默认处理（避免行为异常）
            QTreeView *tv = qobject_cast<QTreeView *>(treeView);
            QModelIndex cur = treeView->currentIndex();
            QModelIndex below = tv ? tv->indexBelow(cur) : QModelIndex();
            if (below.isValid())
            {
                return false;
            }
            return true;
        }
    }

    // 先在当前区域内尝试移动，若失败则尝试跨区域切换
    if (moveFocusInRegion(region, static_cast<Qt::Key>(event->key()), focused))
    {
        return true;
    }
    moveFocusToAdjacentRegion(region, static_cast<Qt::Key>(event->key()));
    return true;  // 方向键统一由我们处理
}

QWidget *KeyNavigation::firstFocusable()
{
    // 优先聚焦中间内容区，其次左侧导航
    QWidget *first = firstFocusableIn(NavRegion::OverviewStack);
    if (!first)
    {
        first = firstFocusableIn(NavRegion::Navigations);
    }
    return first;
}

QList<QWidget *> KeyNavigation::focusableChildrenByVertical(QWidget *parent) const
{
    QList<QWidget *> result;
    if (!parent || parent != m_ui->widgetNavigations)
    {
        return result;
    }
    // 按 UI 设计顺序：应用、最近文件、运行命令、搜索文件、主目录、设置、系统监控、电源
    QWidget *buttons[] = {
        m_ui->btnAppsOverview, m_ui->btnRecentFilesOverview, m_ui->btnRunCommand,
        m_ui->btnSearchFiles,  m_ui->btnHomeDir,            m_ui->btnSettings,
        m_ui->btnSystemMonitor, m_ui->btnPower};
    for (QWidget *w : buttons)
    {
        if (w && w->isVisible() && w->focusPolicy() != Qt::NoFocus)
        {
            result.append(w);
        }
    }
    return result;
}

QList<QWidget *> KeyNavigation::focusableChildrenByGrid(QGridLayout *layout) const
{
    QList<QWidget *> result;
    if (!layout)
    {
        return result;
    }
    // 按行列顺序收集可聚焦控件，保证方向键移动符合网格布局
    QMap<int, QMap<int, QWidget *>> grid;
    for (int i = 0; i < layout->count(); ++i)
    {
        QLayoutItem *item = layout->itemAt(i);
        if (!item || !item->widget())
        {
            continue;
        }
        QWidget *w = item->widget();
        if (!w->isVisible() || w->focusPolicy() == Qt::NoFocus)
        {
            continue;
        }
        int row, col, rowSpan, colSpan;
        layout->getItemPosition(i, &row, &col, &rowSpan, &colSpan);
        grid[row][col] = w;
    }
    QList<int> rows = grid.keys();
    std::sort(rows.begin(), rows.end());
    for (int row : rows)
    {
        QList<int> cols = grid[row].keys();
        std::sort(cols.begin(), cols.end());
        for (int col : cols)
        {
            result.append(grid[row][col]);
        }
    }
    return result;
}

bool KeyNavigation::moveFocusInRegion(NavRegion region, Qt::Key key, QWidget *focused)
{
    // 左侧导航：上下键在按钮间垂直移动
    if (region == NavRegion::Navigations)
    {
        QList<QWidget *> list = focusableChildrenByVertical(m_ui->widgetNavigations);
        int idx = list.indexOf(focused);
        if (idx < 0)
        {
            return false;
        }
        if (key == Qt::Key_Up && idx > 0)
        {
            list.at(idx - 1)->setFocus();
            return true;
        }
        if (key == Qt::Key_Down && idx < list.size() - 1)
        {
            list.at(idx + 1)->setFocus();
            return true;
        }
        return false;
    }

    // 中间内容区：搜索框与树形视图之间上下切换
    if (region == NavRegion::OverviewStack)
    {
        QWidget *overview = m_ui->widgetOverviewStack->currentWidget();
        if (!overview)
        {
            return false;
        }
        QLineEdit *search = overview->findChild<QLineEdit *>(QStringLiteral("edit_search"));
        QAbstractItemView *tree = overviewTreeView();
        QList<QWidget *> list;
        if (search)
        {
            list.append(search);
        }
        if (tree)
        {
            list.append(tree);
        }
        if (key == Qt::Key_Up)
        {
            if (focused == tree && search)
            {
                search->setFocus();
                return true;
            }
            return false;
        }
        if (key == Qt::Key_Down)
        {
            if (focused == search && tree)
            {
                tree->setFocus();
                return true;
            }
            return false;
        }
        return false;
    }

    // 热门应用：左右键在一行内移动（单行水平布局）
    if (region == NavRegion::Popular)
    {
        QList<QWidget *> list = focusableChildrenByGrid(m_ui->gridLayoutPopularApp);
        int idx = list.indexOf(focused);
        if (idx < 0)
        {
            return false;
        }
        if (key == Qt::Key_Left && idx > 0)
        {
            list.at(idx - 1)->setFocus();
            return true;
        }
        if (key == Qt::Key_Right && idx < list.size() - 1)
        {
            list.at(idx + 1)->setFocus();
            return true;
        }
        return false;
    }

    // 收藏应用：4 列网格，支持上下左右四方向移动
    if (region == NavRegion::Favorite)
    {
        QList<QWidget *> list = focusableChildrenByGrid(m_ui->gridLayoutFavoriteApp);
        int idx = list.indexOf(focused);
        if (idx < 0)
        {
            return false;
        }
        int colMax = 4;  // 每行 4 个应用图标
        int row = idx / colMax;
        int col = idx % colMax;
        int rowsTotal = (list.size() + colMax - 1) / colMax;
        if (key == Qt::Key_Up && row > 0)
        {
            list.at((row - 1) * colMax + col)->setFocus();
            return true;
        }
        if (key == Qt::Key_Down && row < rowsTotal - 1 && (row + 1) * colMax + col < list.size())
        {
            list.at((row + 1) * colMax + col)->setFocus();
            return true;
        }
        if (key == Qt::Key_Left && col > 0)
        {
            list.at(idx - 1)->setFocus();
            return true;
        }
        if (key == Qt::Key_Right && col < colMax - 1 && idx + 1 < list.size())
        {
            list.at(idx + 1)->setFocus();
            return true;
        }
        return false;
    }

    return false;
}

bool KeyNavigation::moveFocusToAdjacentRegion(NavRegion from, Qt::Key key)
{
    QWidget *focused = QApplication::focusWidget();
    if (!focused)
    {
        return false;
    }

    // 辅助 lambda：在目标区域内优先找沿方向最近的控件，否则取该区域第一个可聚焦控件
    auto tryRegion = [&](NavRegion r) -> QWidget *
    {
        QWidget *t = nearestFocusableInDirection(r, focused, key);
        return t ? t : firstFocusableIn(r);
    };

    QWidget *target = nullptr;
    // ========== 跨区域切换规则：根据 from + key 确定目标区域及目标控件 ==========
    // 布局示意：Navigations(左) | OverviewStack(中) | Popular(右上) | Favorite(右下)

    if (from == NavRegion::Navigations && key == Qt::Key_Right)
    {
        // 左侧导航 → 右：进入中间内容区
        target = tryRegion(NavRegion::OverviewStack);
    }
    else if (from == NavRegion::OverviewStack && key == Qt::Key_Left)
    {
        // 中间内容区 → 左：回到左侧导航
        target = tryRegion(NavRegion::Navigations);
    }
    else if (from == NavRegion::OverviewStack && key == Qt::Key_Right)
    {
        // 中间内容区 → 右：进入 Popular 或 Favorite
        // 二者在右侧，横向排列（Popular 上、Favorite 下），合并为候选列表按方向找最近
        QList<QWidget *> candidates = focusableChildrenByGrid(m_ui->gridLayoutPopularApp);
        candidates.append(focusableChildrenByGrid(m_ui->gridLayoutFavoriteApp));
        target = nearestInDirection(candidates, focused, key);
        if (!target)
        {
            target = firstFocusableIn(NavRegion::Popular);
        }
        if (!target)
        {
            target = firstFocusableIn(NavRegion::Favorite);
        }
    }
    else if (from == NavRegion::OverviewStack && key == Qt::Key_Down)
    {
        // 中间内容区 → 下：进入 Favorite（右下），若 Popular 为空则退化为 Popular（右上）
        target = tryRegion(NavRegion::Favorite);
        if (!target)
        {
            target = firstFocusableIn(NavRegion::Popular);
        }
    }
    else if (from == NavRegion::Popular && key == Qt::Key_Left)
    {
        // 热门应用 → 左：回到中间内容区的树形视图
        if (QAbstractItemView *tree = overviewTreeView())
        {
            target = nearestInDirection({tree}, focused, key);
            if (!target)
            {
                target = tree;
            }
        }
    }
    else if (from == NavRegion::Popular && key == Qt::Key_Down)
    {
        // 热门应用 → 下：进入 Favorite（右下）
        target = tryRegion(NavRegion::Favorite);
    }
    else if (from == NavRegion::Favorite && key == Qt::Key_Up)
    {
        // 收藏应用 → 上：回到 Popular（右上）
        target = tryRegion(NavRegion::Popular);
    }
    else if (from == NavRegion::Favorite && key == Qt::Key_Left)
    {
        // 收藏应用 → 左：回到中间内容区
        target = tryRegion(NavRegion::OverviewStack);
    }

    if (target)
    {
        // 判断焦点是否来自树形视图（应用列表/文件树）——focused 可能是 tree 或其 viewport
        QAbstractItemView *fromTree = qobject_cast<QAbstractItemView *>(focused);
        if (!fromTree && focused && focused->parentWidget())
        {
            fromTree = qobject_cast<QAbstractItemView *>(focused->parentWidget());
        }
        // 从 OverviewStack 的树向右/向下切出时，Qt 可能尚未完成树内部状态更新，立即 setFocus 会出问题，需延迟一帧
        bool deferFromTree = (from == NavRegion::OverviewStack &&
                             (key == Qt::Key_Right || key == Qt::Key_Down) && fromTree);
        if (deferFromTree)
        {
            QPointer<QWidget> targetGuard(target);
            QTimer::singleShot(0, m_window, [targetGuard]()
            {
                if (targetGuard && targetGuard->isVisible())
                {
                    targetGuard->setFocus(Qt::TabFocusReason);
                }
            });
        }
        else
        {
            target->setFocus(Qt::TabFocusReason);
        }
        // 若目标为树形视图，根据来源控件的 Y 坐标，在目标树中同步选中对应行的项
        if (QAbstractItemView *treeView = qobject_cast<QAbstractItemView *>(target))
        {
            QPoint fromCenter = focused->mapToGlobal(focused->rect().center());
            int vpCenterX = treeView->viewport()->width() / 2;
            QPoint globalAtY = treeView->viewport()->mapToGlobal(QPoint(vpCenterX, 0));
            globalAtY.setY(fromCenter.y());
            QPoint localPos = treeView->viewport()->mapFromGlobal(globalAtY);
            QModelIndex idx = treeView->indexAt(localPos);
            if (idx.isValid())
            {
                treeView->setCurrentIndex(idx);
            }
        }
        return true;
    }
    return false;
}

KeyNavigation::NavRegion KeyNavigation::regionOf(QWidget *w) const
{
    // 从控件向上遍历父级，找到所属的导航区域容器
    while (w && w != m_window)
    {
        if (w == m_ui->widgetNavigations)
        {
            return NavRegion::Navigations;
        }
        if (w == m_ui->widgetOverviewStack || (w && m_ui->widgetOverviewStack->isAncestorOf(w)))
        {
            return NavRegion::OverviewStack;
        }
        QWidget *popular = m_ui->widgetPopular;
        if (w == popular || (popular && popular->isAncestorOf(w)))
        {
            return NavRegion::Popular;
        }
        if (w == m_ui->scrollAreaFavoriteApp || (w && m_ui->scrollAreaFavoriteApp->isAncestorOf(w)))
        {
            return NavRegion::Favorite;
        }
        w = w->parentWidget();
    }
    return NavRegion::OverviewStack;
}

QAbstractItemView *KeyNavigation::overviewTreeView() const
{
    // 根据当前 Overview 页类型返回对应的树形视图
    QWidget *overview = m_ui->widgetOverviewStack->currentWidget();
    if (AppsOverview *ao = qobject_cast<AppsOverview *>(overview))
    {
        return ao->getAppsView();
    }
    if (RecentFilesOverview *rfo = qobject_cast<RecentFilesOverview *>(overview))
    {
        return rfo->getTreeView();
    }
    return nullptr;
}

QWidget *KeyNavigation::nearestFocusableInDirection(NavRegion region, QWidget *from, Qt::Key key) const
{
    // 收集目标区域内所有可聚焦控件，再从中找沿方向最近的
    QList<QWidget *> candidates;
    if (region == NavRegion::Navigations)
    {
        candidates = focusableChildrenByVertical(m_ui->widgetNavigations);
    }
    else if (region == NavRegion::OverviewStack)
    {
        QWidget *overview = m_ui->widgetOverviewStack->currentWidget();
        if (overview)
        {
            if (QLineEdit *search = overview->findChild<QLineEdit *>(QStringLiteral("edit_search")))
            {
                candidates.append(search);
            }
            if (QAbstractItemView *tree = overviewTreeView())
            {
                candidates.append(tree);
            }
        }
    }
    else if (region == NavRegion::Popular)
    {
        candidates = focusableChildrenByGrid(m_ui->gridLayoutPopularApp);
    }
    else if (region == NavRegion::Favorite)
    {
        candidates = focusableChildrenByGrid(m_ui->gridLayoutFavoriteApp);
    }
    return nearestInDirection(candidates, from, key);
}

QWidget *KeyNavigation::nearestInDirection(const QList<QWidget *> &candidates, QWidget *from, Qt::Key key) const
{
    if (candidates.isEmpty())
    {
        return nullptr;
    }

    // 获取“当前位置”的屏幕中心点；若 from 为树形视图，则用当前选中项的中心
    QPoint fromCenter;
    QAbstractItemView *itemView = qobject_cast<QAbstractItemView *>(from);
    if (!itemView && from && from->parentWidget())
    {
        itemView = qobject_cast<QAbstractItemView *>(from->parentWidget());
    }
    if (itemView && itemView->currentIndex().isValid())
    {
        QRect vr = itemView->visualRect(itemView->currentIndex());
        fromCenter = itemView->viewport()->mapToGlobal(vr.center());
    }
    else
    {
        fromCenter = from->mapToGlobal(from->rect().center());
    }

    // 在所有朝 direction 方向且在有效范围内的候选中，取欧氏距离最近的
    QWidget *best = nullptr;
    double bestDist = std::numeric_limits<double>::max();

    for (QWidget *w : candidates)
    {
        QPoint wCenter = w->mapToGlobal(w->rect().center());

        // 过滤：只考虑在按键方向上的候选
        if (key == Qt::Key_Right && wCenter.x() <= fromCenter.x())
        {
            continue;
        }
        if (key == Qt::Key_Left && wCenter.x() >= fromCenter.x())
        {
            continue;
        }
        if (key == Qt::Key_Down && wCenter.y() <= fromCenter.y())
        {
            continue;
        }
        if (key == Qt::Key_Up && wCenter.y() >= fromCenter.y())
        {
            continue;
        }

        double dx = wCenter.x() - fromCenter.x();
        double dy = wCenter.y() - fromCenter.y();
        double dist = std::sqrt(dx * dx + dy * dy);
        if (dist < bestDist)
        {
            bestDist = dist;
            best = w;
        }
    }
    return best;
}

QWidget *KeyNavigation::firstFocusableIn(NavRegion region) const
{
    // 各区域“第一个”按 UI 顺序：Navigations 首按钮，Overview 搜索框，Popular/Favorite 网格首项
    if (region == NavRegion::Navigations)
    {
        QList<QWidget *> list = focusableChildrenByVertical(m_ui->widgetNavigations);
        return list.isEmpty() ? nullptr : list.first();
    }
    if (region == NavRegion::OverviewStack)
    {
        QWidget *overview = m_ui->widgetOverviewStack->currentWidget();
        if (!overview)
        {
            return nullptr;
        }
        // 搜索框在树形视图上方，优先作为首焦点
        if (QLineEdit *search = overview->findChild<QLineEdit *>(QStringLiteral("edit_search")))
        {
            return search;
        }
        return overviewTreeView();
    }
    if (region == NavRegion::Popular)
    {
        QList<QWidget *> list = focusableChildrenByGrid(m_ui->gridLayoutPopularApp);
        return list.isEmpty() ? nullptr : list.first();
    }
    if (region == NavRegion::Favorite)
    {
        QList<QWidget *> list = focusableChildrenByGrid(m_ui->gridLayoutFavoriteApp);
        return list.isEmpty() ? nullptr : list.first();
    }
    return nullptr;
}

}  // namespace Menu
}  // namespace Kiran
