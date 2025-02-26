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

#include <kiran-integration/theme/palette.h>
#include <qt5-log-i.h>
#include <KActivities/KActivities/ResourceInstance>
#include <KIO/ApplicationLauncherJob>
#include <KIOCore/KFileItem>
#include <KService/KService>
#include <KWindowSystem>
#include <QColor>
#include <QDesktopServices>
#include <QFileInfo>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

#include "app-button.h"
#include "app-group.h"
#include "ks-i.h"
#include "lib/common/logging-category.h"
#include "lib/common/utility.h"
#include "lib/common/window-info-helper.h"
#include "plugin-i.h"

const int APP_BUTTON_HEIGHT = 32;

namespace Kiran
{
namespace Taskbar
{
AppButton::AppButton(IAppletImport *import, QWidget *parent)
    : StyledButton(parent),
      m_import(import)
{
    auto *appGroup = (AppGroup *)parent;
    connect(appGroup, &AppGroup::windowChanged, this, &AppButton::changedWindow);
    connect(appGroup, &AppGroup::moveGroupStarted, this, &AppButton::setDragFlag);

    connect(this, &QAbstractButton::clicked, this, &AppButton::buttonClicked);

    setIconSize(QSize(PANEL_APP_ICON_SIZE, PANEL_APP_ICON_SIZE));
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
}

void AppButton::setAppInfo(const AppInfo &appInfo)
{
    m_appBaseInfo = appInfo;
    getInfoFromUrl();
}

void AppButton::setAppInfo(const AppInfo &appInfo, const WId &wid)
{
    m_appBaseInfo = appInfo;
    m_wid = wid;

    if (m_appBaseInfo.m_url.isEmpty())
    {
        // 找不到 desktop file 的app
        // 使用默认图标
        QPixmap icon = KWindowSystem::icon(wid, 25, 25, true);
        setIcon(QIcon(icon));

        // 获取名称
        QString visibleName = WindowInfoHelper::getAppNameByWId(wid);
        setToolTip(visibleName);
    }
    else
    {
        getInfoFromUrl();
    }
}

void AppButton::getInfoFromUrl()
{
    if (m_appBaseInfo.m_url.isEmpty())
    {
        return;
    }

    KFileItem fileItem(m_appBaseInfo.m_url);
    if (fileItem.isNull())
    {
        KLOG_WARNING(LCTaskbar) << "get url info failed, url:" << m_appBaseInfo.m_url;
        return;
    }

    KLOG_INFO() << "AppButton getInfoFromUrl" << fileItem.iconName() << fileItem.mimeComment() << fileItem.name();

    QIcon icon = QIcon::fromTheme(fileItem.iconName());
    if (icon.isNull())
    {
        // 支持某些desktop文件不规范的情况，如 icon=xx.png
        icon = QIcon::fromTheme(QFileInfo(fileItem.iconName()).baseName());
    }

    setIcon(icon);  // 图标正确，除了桌面的计算机、主文件夹、回收站等
    if (fileItem.isDesktopFile())
    {
        setToolTip(fileItem.mimeComment());
    }
    // 普通文件
    else
    {
        setToolTip(fileItem.name());
    }
}

bool AppButton::checkDropAccept(QPoint pos)
{
    // 位于中间2/4位置，可以接受drop
    Qt::AlignmentFlag alignment = getLayoutAlignment();
    QRect rect = geometry();
    if (rect.contains(pos))
    {
        QRect rectCheck;
        if (Qt::AlignLeft == alignment)
        {
            rectCheck = rect.adjusted(rect.width() / 4, 0, -rect.width() / 4, 0);
        }
        else
        {
            rectCheck = rect.adjusted(0, rect.height() / 4, 0, -rect.height() / 4);
        }

        if (rectCheck.contains(pos))
        {
            return true;
        }

        return false;
    }

    return false;
}

Qt::AlignmentFlag AppButton::getLayoutAlignment()
{
    int orientation = m_import->getPanel()->getOrientation();
    Qt::AlignmentFlag alignment =
        (orientation == PanelOrientation::PANEL_ORIENTATION_BOTTOM ||
         orientation == PanelOrientation::PANEL_ORIENTATION_TOP)
            ? Qt::AlignLeft
            : Qt::AlignTop;

    return alignment;
}

void AppButton::setUrl(QUrl url)
{
    m_appBaseInfo.m_url = std::move(url);
    getInfoFromUrl();
}

void AppButton::setShowVisualName(const bool &isShow)
{
    m_isShowName = isShow;

    updateShowName();
}

void AppButton::reset()
{
    m_wid = 0;
    getInfoFromUrl();
    updateShowName();
}

void AppButton::setDragFlag(bool flag)
{
    m_pressed = false;
    m_dragFlag = flag;
}

void AppButton::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu;
    bool check_result = false;

    if (0 == m_wid)
    {
        menu.addAction(tr("Run app"), this, [=]()
                       {
                           buttonClicked();
                       });
    }
    else
    {
        menu.addAction(tr("Close all windows"), this,
                       [=]()
                       {
                           emit windowCloseAll();
                       });
    }

    emit isInFavorite(m_appBaseInfo.m_url.fileName(), check_result);
    if (!check_result)
    {
        menu.addAction(tr("Add to favorite"), this, [=]()
                       {
                           emit addToFavorite(m_appBaseInfo.m_url.fileName());
                       });
    }
    else
    {
        menu.addAction(tr("Remove from favorite"), this, [=]()
                       {
                           emit removeFromFavorite(m_appBaseInfo.m_url.fileName());
                       });
    }

    emit isInTasklist(m_appBaseInfo.m_url, check_result);
    if (!check_result)
    {
        menu.addAction(tr("Add to tasklist"), this,
                       [=]()
                       {
                           emit addToTasklist(m_appBaseInfo.m_url, this);
                       });
    }
    else
    {
        menu.addAction(tr("Remove from tasklist"), this,
                       [=]()
                       {
                           emit removeFromTasklist(m_appBaseInfo.m_url);
                       });
    }

    // 自带菜单
    KService::Ptr service = KService::serviceByMenuId(m_appBaseInfo.m_url.fileName());
    if (service.data())
    {
        bool firstAdd = true;
        for (const KServiceAction &serviceAction : service->actions())
        {
            if (serviceAction.noDisplay())
            {
                continue;
            }

            if (firstAdd)
            {
                menu.addSeparator();
                firstAdd = false;
            }
            QAction *action = menu.addAction(
                QIcon::fromTheme(serviceAction.icon()), serviceAction.text(), this,
                [=]()
                {
                    auto *job = new KIO::ApplicationLauncherJob(serviceAction);
                    job->start();

                    // 通知kactivitymanagerd
                    KActivities::ResourceInstance::notifyAccessed(
                        QUrl(QStringLiteral("applications:") + service->storageId()));
                });
            if (serviceAction.isSeparator())
            {
                action->setSeparator(true);
            }
        }
    }

    menu.exec(mapToGlobal(event->pos()));
    update();
}

void AppButton::enterEvent(QEvent *event)
{
    StyledButton::enterEvent(event);

    // 鼠标移入，显示预览窗口
    if (0 == m_wid)
    {
        return;
    }

    emit previewerShow(m_wid);

    QToolButton::enterEvent(event);
}

void AppButton::leaveEvent(QEvent *event)
{
    StyledButton::leaveEvent(event);

    // 鼠标移出，隐藏预览窗口
    if (0 == m_wid)
    {
        return;
    }

    emit previewerHide(m_wid);
    QToolButton::leaveEvent(event);
}

void AppButton::paintEvent(QPaintEvent *event)
{
    // 图标 + 文字
    // 在底部面板为40的情况下，进行等比计算
    // 宽=40×4=160
    // 高=32
    // 图标+文字=136
    // 图标文字间隔=8
    auto panelSize = m_import->getPanel()->getSize();
    const int designWidth = panelSize * 4;
    const int designHeight = APP_BUTTON_HEIGHT;
    const int designIconTextMargin = 8;
    const int designIconTextWidth = designWidth - APP_BUTTON_HEIGHT;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);  // 设置反走样，使边缘平滑

    auto *palette = Kiran::Theme::Palette::getDefault();

    QPainterPath path;
    path.addRoundedRect(rect(), 4, 4);

    // 背景绘制
    QColor bgColor;
    if (isChecked())
    {
        // 选中
        bgColor = palette->getColor(Kiran::Theme::Palette::SUNKEN,
                                    Kiran::Theme::Palette::WIDGET);
        painter.setBrush(bgColor);
    }

    if (m_pressed)
    {
        // 点击
        bgColor = palette->getColor(Kiran::Theme::Palette::SUNKEN,
                                    Kiran::Theme::Palette::WIDGET);
        painter.setBrush(bgColor);
    }
    else if (m_hovered)
    {
        // 悬停
        bgColor = palette->getColor(Kiran::Theme::Palette::MOUSE_OVER,
                                    Kiran::Theme::Palette::WIDGET);
        painter.setBrush(bgColor);
    }
    else
    {
        // 正常
        bgColor = Qt::transparent;
    }

    QPen pen = painter.pen();
    painter.setPen(Qt::NoPen);

    painter.drawPath(path);

    painter.setPen(pen);

    // 图标文字绘制
    if (text().isEmpty())
    {
        // 仅图标
        int iconX = (width() - iconSize().width()) / 2;
        int iconY = (height() - iconSize().height()) / 2;
        QPixmap pixmap = icon().pixmap(iconSize());
        painter.drawPixmap(iconX, iconY, iconSize().width(), iconSize().height(), pixmap);
    }
    else
    {
        int iconX = (width() - designIconTextWidth) / 2;
        int iconY = (height() - iconSize().height()) / 2;
        QPixmap pixmap = icon().pixmap(iconSize());
        painter.drawPixmap(iconX, iconY, iconSize().width(), iconSize().height(), pixmap);

        int textX = iconX + iconSize().width() + designIconTextMargin;
        int textWidth = designIconTextWidth - designIconTextMargin - iconSize().width();

        painter.drawText(textX, iconY, textWidth, iconSize().height(),
                         Qt::AlignLeft | Qt::AlignVCenter, text());
    }

    // 底部横条
    int relationAppSize = 0;
    emit getRelationAppSize(relationAppSize);
    if (0 == relationAppSize)
    {
        return;
    }

    const int rectHeight = 2;  // 高度为 2

    QRect bottomRect = rect();
    if (text().isEmpty())
    {
        // 在底部面板为40的情况下，进行等比计算
        // 横条宽为 按钮宽/2
        bottomRect.adjust(rect().width() / 4, rect().height() - rectHeight,
                          -rect().width() / 4, 0);
    }
    else
    {
        int adjustX = (width() - designIconTextWidth) / 2;
        bottomRect.adjust(adjustX, rect().height() - rectHeight, -adjustX, 0);
    }

    bgColor = palette->getColor(Kiran::Theme::Palette::SELECTED,
                                Kiran::Theme::Palette::WIDGET);

    painter.fillRect(bottomRect, bgColor);

    // 如果有多个关联窗口，右侧增加覆盖层
    if (1 == relationAppSize)
    {
        return;
    }

    bgColor = palette->getColor(Kiran::Theme::Palette::ACTIVE,
                                Kiran::Theme::Palette::WIDGET);

    bgColor.setAlpha(125);

    if (!m_hovered && !isChecked())
    {
        // 横线短一点
        bottomRect.adjust(bottomRect.width() / 2, 0, 0, 0);
    }
    else
    {
        bottomRect.adjust(bottomRect.width() / 5 * 4, 0, 0, 0);
    }

    painter.fillRect(bottomRect, bgColor);
}

void AppButton::mousePressEvent(QMouseEvent *event)
{
    emit mousePressed(event);

    StyledButton::mousePressEvent(event);
}

void AppButton::mouseMoveEvent(QMouseEvent *event)
{
    emit mouseMoved(event);

    StyledButton::mouseMoveEvent(event);
}

void AppButton::mouseReleaseEvent(QMouseEvent *event)
{
    emit mouseReleased(event);

    if (m_dragFlag)
    {
        m_dragFlag = false;
        return;
    }

    StyledButton::mouseReleaseEvent(event);
}

void AppButton::updateLayout()
{
    //    updateName();
}

void AppButton::changedWindow(WId wid, NET::Properties properties,
                              NET::Properties2 properties2)
{
    if (m_wid != wid)
    {
        return;
    }

    if (properties.testFlag(NET::WMState))
    {
        if (WindowInfoHelper::hasState(wid, NET::DemandsAttention))
        {
            // TODO: 提醒 需要样式支持
        }
        else if (WindowInfoHelper::hasState(wid, NET::Focused))
        {
            // TODO: 已聚焦窗口，清除提醒 需要样式支持
        }
    }

    if (properties.testFlag(NET::WMName))
    {
        updateShowName();
    }
}

void AppButton::updateShowName()
{
    auto panelSize = m_import->getPanel()->getSize();
    int height = APP_BUTTON_HEIGHT;

    if (0 != m_wid)
    {
        m_visualName = WindowInfoHelper::getAppNameByWId(m_wid);
        setToolTip(m_visualName);

        int orientation = m_import->getPanel()->getOrientation();

        if (m_isShowName &&
            (orientation == PanelOrientation::PANEL_ORIENTATION_BOTTOM ||
             orientation == PanelOrientation::PANEL_ORIENTATION_TOP))
        {
            setFixedSize(panelSize * 4, height);
            QString elideText =
                Utility::getElidedText(fontMetrics(), m_visualName, height * 3);
            setText(elideText);
            return;
        }
    }

    setFixedSize(height, height);
    setText("");
}

void AppButton::buttonClicked()
{
    // 如果没有关联的窗口，则启动应用（固定到任务栏的应用）
    // 如果只有一个关联的窗口，则激活或最小化窗口
    // 如果由多个关联窗口，不用处理
    int relationAppSize = 0;
    emit getRelationAppSize(relationAppSize);

    if (0 == relationAppSize)
    {
        KFileItem fileItem(m_appBaseInfo.m_url);
        if (fileItem.isNull())
        {
            KLOG_WARNING(LCTaskbar) << "get url info failed, url:" << m_appBaseInfo.m_url;
            return;
        }

        if (fileItem.isDesktopFile())
        {
            KService::Ptr service =
                KService::serviceByStorageId(m_appBaseInfo.m_url.fileName());
            // 启动应用
            auto *job = new KIO::ApplicationLauncherJob(service);
            job->start();

            // 通知kactivitymanagerd
            KActivities::ResourceInstance::notifyAccessed(
                QUrl(QStringLiteral("applications:") + service->storageId()));
        }
        else
        {
            bool ret = QDesktopServices::openUrl(m_appBaseInfo.m_url);
            if (!ret)
            {
                KLOG_WARNING(LCTaskbar) << "start programe failed，url:" << m_appBaseInfo.m_url;
            }
        }
    }
    else if (1 == relationAppSize)
    {
        if (WindowInfoHelper::isActived(m_wid))
        {
            WindowInfoHelper::minimizeWindow(m_wid);
        }
        else
        {
            WindowInfoHelper::activateWindow(m_wid);
        }
    }
    else
    {
        // 显示或隐藏预览
        emit previewerShowChange(m_wid);

        // 点击后check状态会变化，需要维持之前的状态
        setChecked(!isChecked());
    }
}

}  // namespace Taskbar

}  // namespace Kiran
