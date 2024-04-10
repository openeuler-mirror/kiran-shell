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
 * Author:     yangfeng <yangfeng@kylinsec.com.cn>
 */

#include <kiran-style/style-palette.h>
#include <ks-i.h>
#include <plugin-i.h>
#include <qt5-log-i.h>
#include <KActivities/KActivities/ResourceInstance>
#include <KDesktopFile>
#include <KIO/ApplicationLauncherJob>
#include <KIOCore/KFileItem>
#include <KService/KService>
#include <KWindowSystem>
#include <QDesktopServices>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QProcess>
#include <QSettings>

#include "app-button.h"
#include "app-group.h"
#include "lib/common/define.h"
#include "lib/common/setting-process.h"
#include "lib/common/utility.h"
#include "lib/common/window-info-helper.h"

namespace Kiran
{
namespace Taskbar
{
AppButton::AppButton(IAppletImport *import, QWidget *parent)
    : StyledButton(parent),
      m_import(import),
      m_wid(0),
      m_isShowName(false)
{
    AppGroup *appGroup = (AppGroup *)parent;
    connect(appGroup, &AppGroup::windowChanged, this, &AppButton::changedWindow);

    connect(this, &QAbstractButton::clicked, this, &AppButton::buttonClicked);

    auto panelSize = m_import->getPanel()->getSize();
    int iconSize = panelSize - BUTTON_BLANK_SPACE * 2;
    setIconSize(QSize(iconSize, iconSize));
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
}

void AppButton::setAppInfo(const AppBaseInfo &appBaseInfo)
{
    m_appBaseInfo = appBaseInfo;
    getInfoFromUrl();
}

void AppButton::setAppInfo(const QByteArray &wmClass, const WId &wid)
{
    m_appBaseInfo.m_url = WindowInfoHelper::getUrlByWId(wid);

    //    KLOG_INFO() << "AppButton::setAppInfo" << wmClass << wid << m_appBaseInfo.m_url;
    m_appBaseInfo.m_wmClass = wmClass;
    m_wid = wid;

    //    KLOG_INFO() << "desktop file:" << m_desktopFile;
    if (m_appBaseInfo.m_url.isEmpty())
    {
        // 找不到 desktop file 的app
        // 使用默认图标
        QPixmap icon = KWindowSystem::icon(wid);
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
        KLOG_ERROR() << "get url info failed, url:" << m_appBaseInfo.m_url;
        return;
    }

    //    KLOG_INFO() << "AppButton::getInfoFromUrl" << m_appBaseInfo.m_url << fileItem.iconName() << fileItem.mimeComment();

    setIcon(QIcon::fromTheme(fileItem.iconName()));  // 图标正确，除了桌面的计算机、主文件夹、回收站等
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

void AppButton::setUrl(QUrl url)
{
    m_appBaseInfo.m_url = url;
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

void AppButton::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu;
    bool check_result = false;

    if (0 == m_wid)
    {
        menu.addAction(tr("Run app"), this, [=]()
                       { buttonClicked(); });
    }
    else
    {
        menu.addAction(tr("Close all windows"), this, [=]()
                       { emit windowClose(m_wid); });
    }

    emit isInFavorite(m_appBaseInfo.m_url.fileName(), check_result);
    if (!check_result)
    {
        menu.addAction(tr("Add to favorite"), this, [=]()
                       { emit addToFavorite(m_appBaseInfo.m_url.fileName()); });
    }
    else
    {
        menu.addAction(tr("Remove from favorite"), this, [=]()
                       { emit removeFromFavorite(m_appBaseInfo.m_url.fileName()); });
    }

    emit isInTasklist(m_appBaseInfo.m_url, check_result);
    if (!check_result)
    {
        menu.addAction(tr("Add to tasklist"), this, [=]()
                       { emit addToTasklist(m_appBaseInfo.m_url, this); });
    }
    else
    {
        menu.addAction(tr("Remove from tasklist"), this, [=]()
                       { emit removeFromTasklist(m_appBaseInfo.m_url); });
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

    emit previewerShowChange(m_wid);
}

void AppButton::leaveEvent(QEvent *event)
{
    StyledButton::leaveEvent(event);

    // 鼠标移出，隐藏预览窗口
    if (0 == m_wid)
    {
        return;
    }

    emit previewerShowChange(m_wid);
}

void AppButton::paintEvent(QPaintEvent *event)
{
    StyledButton::paintEvent(event);

    int relationAppSize = 0;
    emit getRelationAppSize(relationAppSize);
    if (0 == relationAppSize)
    {
        return;
    }
    auto stylePalette = Kiran::StylePalette::instance();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);  // 设置反走样，使边缘平滑

    // 底部横线
    const int rectHeight = 3;  // 高度为 2

    QRect bottomRect = rect();
    bottomRect.adjust(0, rect().height() - rectHeight, 0, 0);
    if (!m_hovered && !isChecked())
    {
        // 横线短一点
        bottomRect.adjust(5, 0, -5, 0);
    }
    QColor bgColor = stylePalette->color(Kiran::StylePalette::ColorState::Checked,
                                         Kiran::StylePalette::WidgetType::Widget,
                                         Kiran::StylePalette::WidgetColorRule::Background);
    painter.fillRect(bottomRect, bgColor);

    // 如果有多个关联窗口，右侧增加覆盖层
    if (1 == relationAppSize)
    {
        return;
    }
    bgColor = stylePalette->color(Kiran::StylePalette::ColorState::Active,
                                  Kiran::StylePalette::WidgetType::Widget,
                                  Kiran::StylePalette::WidgetColorRule::Border);
    bgColor.setAlpha(125);

    if (!m_hovered && !isChecked())
    {
        // 横线短一点
        bottomRect.adjust(bottomRect.width() - 10, 0, 0, 0);
    }
    else
    {
        // 整个右侧
        //bottomRect = rect();
        //bottomRect.adjust(rect().width() - 4, 0, 0, 0);

        bottomRect.adjust(bottomRect.width() - 5, 0, 0, 0);
    }

    QPen pen = painter.pen();
    pen.setColor(bgColor);
    painter.setPen(pen);
    painter.fillRect(bottomRect, bgColor);
}

void AppButton::updateLayout()
{
    //    updateName();
}

void AppButton::changedWindow(WId wid, NET::Properties properties, NET::Properties2 properties2)
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
    if (0 != m_wid)
    {
        m_visualName = WindowInfoHelper::getAppNameByWId(m_wid);
        setToolTip(m_visualName);

        auto size = m_import->getPanel()->getSize();
        int orientation = m_import->getPanel()->getOrientation();

        if (m_isShowName &&
            (orientation == PanelOrientation::PANEL_ORIENTATION_BOTTOM ||
             orientation == PanelOrientation::PANEL_ORIENTATION_TOP))
        {
            setFixedSize(size * 3, size);
            QString elideText = Utility::getElidedText(fontMetrics(), m_visualName, size * 2);
            setText(elideText);
            return;
        }
    }

    auto size = m_import->getPanel()->getSize();
    setFixedSize(size, size);
    setText("");
    return;
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
            KLOG_ERROR() << "get url info failed, url:" << m_appBaseInfo.m_url;
            return;
        }

        if (fileItem.isDesktopFile())
        {
            KService::Ptr service = KService::serviceByStorageId(m_appBaseInfo.m_url.fileName());
            //启动应用
            auto *job = new KIO::ApplicationLauncherJob(service);
            job->start();

            //通知kactivitymanagerd
            KActivities::ResourceInstance::notifyAccessed(QUrl(QStringLiteral("applications:") + service->storageId()));
        }
        else
        {
            bool ret = QDesktopServices::openUrl(m_appBaseInfo.m_url);
            if (!ret)
            {
                KLOG_ERROR() << "start programe failed，url:" << m_appBaseInfo.m_url;
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
