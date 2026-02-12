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

#pragma once

#include <QObject>

class QAbstractItemView;
class QGridLayout;
class QKeyEvent;
class QWidget;

namespace Ui
{
class Window;
}

namespace Kiran
{
namespace Menu
{
class Window;

/**
 * @brief Menu 窗口的键盘导航管理器
 *
 * 负责应用菜单窗口内的焦点切换逻辑，支持方向键在以下区域间导航：
 * - 左侧导航栏（应用、最近文件、运行命令等）
 * - 中间内容区（ overview  stacks：应用列表 / 最近文件，含搜索框和树形视图）
 * - 右侧热门应用 / 收藏应用网格
 *
 * 导航规则：
 * - 区域内：上下/左右键在可聚焦控件间移动
 * - 跨区域：根据方向键切换到相邻区域中最近的可聚焦控件
 */
class KeyNavigation : public QObject
{
    Q_OBJECT
public:
    explicit KeyNavigation(Window *window, Ui::Window *ui);
    ~KeyNavigation() override = default;

    /** 处理方向键：区域内移动或跨区域切换焦点，返回 true 表示已消费该按键 */
    bool handleKeyPress(QKeyEvent *event);
    /** 返回初始应获得焦点的控件（如 OverviewStack 的搜索框或应用列表） */
    QWidget *firstFocusable();

private:
    /** 可导航区域枚举，对应 UI 布局的各个区块 */
    enum class NavRegion
    {
        Navigations,   ///< 左侧导航按钮（应用、最近文件、运行命令等）
        OverviewStack, ///< 中间内容区（搜索框 + 应用/文件树）
        Popular,       ///< 右侧热门应用网格
        Favorite       ///< 右侧收藏应用网格
    };

    /** 按垂直顺序获取可聚焦子控件（用于 Navigations 区域） */
    QList<QWidget *> focusableChildrenByVertical(QWidget *parent) const;
    /** 按网格行列顺序获取可聚焦子控件（用于 Popular/Favorite） */
    QList<QWidget *> focusableChildrenByGrid(QGridLayout *layout) const;
    /** 在指定区域内按方向键移动焦点，成功返回 true */
    bool moveFocusInRegion(NavRegion region, Qt::Key key, QWidget *focused);
    /** 从当前区域按方向键切换到相邻区域 */
    bool moveFocusToAdjacentRegion(NavRegion from, Qt::Key key);
    /** 判断给定控件所属的导航区域 */
    NavRegion regionOf(QWidget *w) const;
    /** 返回指定区域内第一个可聚焦控件 */
    QWidget *firstFocusableIn(NavRegion region) const;
    /** 在指定区域内沿方向查找距离当前焦点最近的控件 */
    QWidget *nearestFocusableInDirection(NavRegion region, QWidget *from, Qt::Key key) const;
    /** 在候选列表中沿方向查找距离 from 最近的控件（基于屏幕坐标） */
    QWidget *nearestInDirection(const QList<QWidget *> &candidates, QWidget *from, Qt::Key key) const;
    /** 获取当前 Overview 页中的树形视图（应用列表或文件树） */
    QAbstractItemView *overviewTreeView() const;

    Window *m_window;   ///< Menu 主窗口
    Ui::Window *m_ui;   ///< 窗口 UI 布局引用
};

}  // namespace Menu
}  // namespace Kiran
