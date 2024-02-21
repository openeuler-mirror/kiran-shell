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

#include <ks-i.h>
#include <plugin-i.h>
#include <qt5-log-i.h>
#include <KActivities/KActivities/ResourceInstance>
#include <KService/KService>
#include <KWindowSystem>
#include <QMenu>
#include <QMouseEvent>
#include <QProcess>
#include <QSettings>

#include "app-button-container.h"
#include "app-button.h"
#include "lib/common/common.h"
#include "lib/common/define.h"
#include "lib/common/window-info-helper.h"

namespace Kiran
{
namespace Taskbar
{
AppButton::AppButton(IAppletImport *import, AppButtonContainer *parent)
    : m_import(import)
{
    connect(parent, &AppButtonContainer::windowAdded, this, &AppButton::addWindow);
    connect(parent, &AppButtonContainer::windowRemoved, this, &AppButton::removeWindow);
    connect(parent, &AppButtonContainer::windowChanged, this, &AppButton::changedWindow);

    connect(this, &QPushButton::clicked, this, &AppButton::buttonClicked);

    auto panelSize = m_import->getPanel()->getSize();
    int iconSize = panelSize - BUTTON_BLANK_SPACE * 2;
    setIconSize(QSize(iconSize, iconSize));

    // QSettings 保存时，会删除原有文件，重新创建一个新文件，所以不能监视文件，此处监视文件夹
    m_settingFileWatcher.addPath(QFileInfo(KIRAN_SHELL_SETTING_FILE).dir().path());
    connect(&m_settingFileWatcher, &QFileSystemWatcher::directoryChanged, this, [=]()
            { updateName(); });

    m_appPreviewer = new AppPreviewer(m_import);
    connect(m_appPreviewer, &AppPreviewer::closeWindow, this, &AppButton::closeWindow);
}

void AppButton::setAppInfo(QByteArray wmClass, WId wid)
{
    m_desktopFile = WindowInfoHelper::getDesktopFileByWId(wid);

    KLOG_INFO() << "AppButton::setAppInfo" << wmClass << m_desktopFile;
    m_wmClass = wmClass;

    //    KLOG_INFO() << "desktop file:" << m_desktopFile;
    if (m_desktopFile.isEmpty())
    {
        // 找不到 desktop file 的app
        // 获取名称
        m_name = WindowInfoHelper::getAppNameByWId(wid);

        // 使用默认图标
        QPixmap icon = KWindowSystem::icon(wid);
        // setIcon(QIcon::fromTheme("application-x-executable"));
        setIcon(QIcon(icon));
        KLOG_INFO() << "app icon:" << icon;

        setToolTip(m_name);
        updateName();
    }
    else
    {
        setAppInfo(m_desktopFile);
    }
}

void AppButton::setAppInfo(QString appId)
{
    // 只传desktopfile，用于固定到任务栏的应用（未打开前）
    m_desktopFile = appId.toLocal8Bit();
    auto service = KService::serviceByStorageId(m_desktopFile);
    if (!service->isValid())
    {
        KLOG_WARNING() << "KService::serviceByStorageId failed";
        return;
    }

    m_name = service->name();
    setIcon(QIcon::fromTheme(service->icon()));
    KLOG_INFO() << "app icon:" << service->icon();

    setToolTip(m_name);
    updateName();
}

void AppButton::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu;
    bool check_result = false;

    if (m_windowId.isEmpty())
    {
        menu.addAction(tr("Run app"), this, [=]()
                       { buttonClicked(); });
    }
    else
    {
        menu.addAction(tr("Close all windows"), this, [=]()
                       { closeAppButton(); });
    }

    emit isInFavorite(m_desktopFile, check_result);
    if (!check_result)
    {
        menu.addAction(tr("Add to favorite"), this, [=]()
                       { emit addToFavorite(m_desktopFile); });
    }
    else
    {
        menu.addAction(tr("Remove from favorite"), this, [=]()
                       { emit removeFromFavorite(m_desktopFile); });
    }

    emit isInTasklist(m_desktopFile, check_result);
    if (!check_result)
    {
        menu.addAction(tr("Add to tasklist"), this, [=]()
                       { emit addToTasklist(m_desktopFile); });
    }
    else
    {
        menu.addAction(tr("Remove from tasklist"), this, [=]()
                       { emit removeFromTasklist(m_desktopFile); });
    }

    menu.exec(mapToGlobal(event->pos()));
}

void AppButton::enterEvent(QEvent *event)
{
    // 移入鼠标，显示预览窗口
    if (m_windowId.isEmpty())
    {
        return;
    }

    int orientation = m_import->getPanel()->getOrientation();
    auto direction = (orientation == PanelOrientation::PANEL_ORIENTATION_BOTTOM ||
                      orientation == PanelOrientation::PANEL_ORIENTATION_TOP)
                         ? QBoxLayout::Direction::LeftToRight
                         : QBoxLayout::Direction::TopToBottom;

    m_appPreviewer->move(pos());

    m_appPreviewer->show();
}

void AppButton::leaveEvent(QEvent *event)
{
    if (!m_appPreviewer->geometry().contains(QCursor::pos()))
    {
        m_appPreviewer->hide();
    }
}

void AppButton::addWindow(QByteArray wmClass, WId wid)
{
    if (wmClass != m_wmClass || m_windowId.contains(wid))
    {
        return;
    }

    m_windowId.push_back(wid);
    m_appPreviewer->addWindow(wid);
}

void AppButton::removeWindow(QByteArray wmClass, WId wid)
{
    if (wmClass != m_wmClass)
    {
        return;
    }

    m_windowId.removeAll(wid);

    if (m_windowId.isEmpty())
    {
        emit windowEmptied();
    }
    m_appPreviewer->removeWindow(wid);
}

void AppButton::changedWindow(WId id, NET::Properties properties, NET::Properties2 properties2)
{
    if (!m_windowId.contains(id))
    {
        return;
    }

    if (properties.testFlag(NET::WMState))
    {
        if (WindowInfoHelper::hasState(id, NET::DemandsAttention))
        {
            // TODO: 提醒 需要样式支持
        }
        else if (WindowInfoHelper::hasState(id, NET::Focused))
        {
            // TODO: 已聚焦窗口，清除提醒 需要样式支持
        }
    }
}

void AppButton::buttonClicked()
{
    // 如果没有关联的窗口，则启动应用（固定到任务栏的应用）
    // 如果只有一个关联的窗口，则激活或最小化窗口
    // 如果由多个关联窗口，不用处理
    if (m_windowId.isEmpty())
    {
        KService::Ptr service = KService::serviceByStorageId(m_desktopFile);

        if (service)
        {
            //启动应用
            QProcess::startDetached(service->exec(), QStringList());

            //通知kactivitymanagerd
            KActivities::ResourceInstance::notifyAccessed(QUrl(QStringLiteral("applications:") + service->storageId()));
        }
    }
    else if (1 == m_windowId.size())
    {
        WId wid = m_windowId.first();
        if (WindowInfoHelper::isActived(wid))
        {
            WindowInfoHelper::minimizeWindow(wid);
        }
        else
        {
            WindowInfoHelper::activeWindow(wid);
        }
    }
}

void AppButton::closeAppButton()
{
    for (WId wid : m_windowId)
    {
        closeWindow(wid);
    }
}

void AppButton::closeWindow(WId wid)
{
    WindowInfoHelper::closeWindow(wid);
    m_appPreviewer->hide();
}

void AppButton::updateName()
{
    auto size = m_import->getPanel()->getSize();
    if (isShowAppBtnTail())
    {
        setFixedSize(size * 3, size);
        QString elideText = getElidedText(fontMetrics(), m_name, size * 2);
        setText(elideText);
    }
    else
    {
        setFixedSize(size, size);
        setText("");
    }
}

}  // namespace Taskbar

}  // namespace Kiran
