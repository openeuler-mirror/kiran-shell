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
#include <QLayoutItem>
#include <QScrollArea>
#include <QVBoxLayout>

#include "lib/common/window-info-helper.h"
#include "lib/common/window-manager.h"
#include "lib/widgets/window-thumbnail.h"
#include "workspace-overview.h"

static const int windowSpacing = 8;
static const int windowMargin = 4;

namespace Kiran
{
namespace Workspace
{
WorkspaceOverview::WorkspaceOverview(int desktop, QWidget *parent)
    : QWidget(parent),
      m_workspaceIndex(desktop)
{
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setFrameStyle(QFrame::NoFrame);  // 无边框
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);  // 禁用水平滚动条

    m_containerWidget = new QWidget();
    m_mainLayout = new QVBoxLayout(m_containerWidget);
    m_mainLayout->setAlignment(Qt::AlignTop);  // 子元素靠上
    m_mainLayout->setContentsMargins(0, 0, 0, 0);

    m_scrollArea->setWidget(m_containerWidget);

    auto *overallLayout = new QVBoxLayout(this);
    overallLayout->addWidget(m_scrollArea);

    connect(&WindowManagerInstance, &Common::WindowManager::windowRemoved, this, &WorkspaceOverview::updateGridLayout);
}

WorkspaceOverview::~WorkspaceOverview() = default;

void WorkspaceOverview::showEvent(QShowEvent *event)
{
    updateGridLayout();
    QWidget::showEvent(event);
}

void WorkspaceOverview::resizeEvent(QResizeEvent *event)
{
    updateGridLayout();
    QWidget::resizeEvent(event);
}

void WorkspaceOverview::updateGridLayout()
{
    // 窗口排列描述
    //    这里将大窗口命名为a, 各个应用窗口命名为b(数组b), b放在a中
    //    规则：b的高一律统一为a的1/3，宽最大为a的1/2，保持b的宽高比，b在a中一行一行排列
    //    累计行宽度，当一行剩余宽度不够放置新的b时，另起一行放置新的b
    //    a支持上下滚动

    if (!isVisible())
    {
        // 涉及到窗口大小设置,如果没有窗口显示,Qt会报警告
        return;
    }

    int bgHeight = height();
    int bgWidth = width();
    int maxSubHeight = bgHeight / 3;  // 子元素的高度为 主窗口 的1/3
    int maxSubWidth = bgWidth / 2;    // 子元素的最大宽度为 主窗口 的1/2

    // 清空布局
    QLayoutItem *item;
    while ((item = m_mainLayout->takeAt(0)) != nullptr)
    {
        delete item;
    }

    updateWindowItem();
    QList<WId> windows = WindowManagerInstance.getAllWindow(m_workspaceIndex);
    // 参照win10,激活的窗口放在最前面
    std::reverse(windows.begin(), windows.end());
    // 添加窗口
    QHBoxLayout *currentRowLayout = nullptr;  // 当前行的布局
    int currentRowWidth = windowMargin * 2;   // 当前行的总宽度
    for (auto window : windows)
    {
        auto *widget = m_windows[window];
        int scaleWidth;
        int scaleHeight;
        int extraWidth;
        int extraHeight;
        widget->getOriginalSize(scaleWidth, scaleHeight, extraWidth, extraHeight);  // 获取原始大小
        float aspectRatio = static_cast<float>(scaleWidth) / scaleHeight;           // 针对要缩放的部分,计算缩放比例

        int subHeight = maxSubHeight - extraHeight;
        int maxSubWidthTemp = maxSubWidth - extraWidth;
        int subWidth = std::min(static_cast<int>(subHeight * aspectRatio), maxSubWidthTemp);  // 根据高度计算宽度
        // 当宽度为最大时,要反推高度
        if (subWidth == maxSubWidthTemp)
        {
            subHeight = subWidth / aspectRatio;
        }
        // 最后加上不可缩放的部分
        subWidth += extraWidth;
        subHeight += extraHeight;
        widget->setFixedSize(subWidth, subHeight);

        // 如果当前行布局为空，创建一个新的 QHBoxLayout
        // 检查当前行是否有足够的宽度来添加新的子元素
        if (nullptr == currentRowLayout || currentRowWidth + subWidth + windowSpacing > bgWidth)
        {
            // 如果没有足够的宽度，结束当前行布局，开始新的行布局
            currentRowLayout = new QHBoxLayout();
            currentRowLayout->setAlignment(Qt::AlignLeft);  // 设置内部控件靠左对齐
            currentRowLayout->setSpacing(windowSpacing);
            currentRowLayout->setContentsMargins(windowMargin, windowMargin, windowMargin, windowMargin);

            m_mainLayout->addLayout(currentRowLayout);
            currentRowWidth = windowMargin * 2;  // 重置当前行宽度
        }

        currentRowLayout->addWidget(widget);          // 添加当前 widget 到行布局
        currentRowWidth += subWidth + windowSpacing;  // 更新当前行的总宽度
    }
    m_containerWidget->adjustSize();
}

void WorkspaceOverview::updateWindowItem()
{
    QList<WId> windows = WindowManagerInstance.getAllWindow(m_workspaceIndex);

    for (auto window : windows)
    {
        if (!m_windows.contains(window))
        {
            m_windows[window] = new WindowThumbnail(window);
        }
    }
    for (auto window : m_windows.keys())
    {
        if (!windows.contains(window))
        {
            delete m_windows.take(window);
        }
    }
}
}  // namespace Workspace
}  // namespace Kiran
