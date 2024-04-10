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
#include <QDrag>
#include <QMimeData>
#include <QMouseEvent>

#include "app-button-container.h"
#include "app-button.h"
#include "app-group.h"
#include "app-previewer.h"
#include "lib/common/define.h"
#include "lib/common/setting-process.h"
#include "lib/common/utility.h"
#include "lib/common/window-info-helper.h"

namespace Kiran
{
namespace Taskbar
{
AppGroup::AppGroup(IAppletImport *import, const AppBaseInfo &appBaseInfo, QWidget *parent)
    : QWidget(parent),
      m_import(import),
      m_buttonFixed(newAppBtn()),
      m_appBaseInfo(appBaseInfo)
{
    init();
}

AppGroup::AppGroup(IAppletImport *import, QWidget *parent)
    : QWidget(parent),
      m_import(import),
      m_buttonFixed(newAppBtn())
{
    m_appBaseInfo.m_isLocked = true;

    init();

    m_buttonFixed->setIcon(QIcon::fromTheme("application-x-executable"));
    m_buttonFixed->show();
}

QUrl AppGroup::getUrl()
{
    return m_appBaseInfo.m_url;
}

bool AppGroup::isLocked()
{
    return m_appBaseInfo.m_isLocked;
}

void AppGroup::setLocked(bool lockFlag)
{
    m_appBaseInfo.m_isLocked = lockFlag;
}

void AppGroup::setDragData(const QUrl &url)
{
    KLOG_INFO() << "AppGroup::setDragData" << url << m_buttonFixed;
    if (m_buttonFixed)
    {
        m_buttonFixed->setUrl(url);
        m_buttonFixed->show();
    }
}

void AppGroup::getRelationAppSize(int &size)
{
    if (SettingProcess::getValue(TASKBAR_SHOW_APP_BTN_TAIL_KEY).toBool() && m_mapWidButton.size() > 0)
    {
        size = 1;
    }
    else
    {
        size = m_mapWidButton.size();
    }
}

void AppGroup::changePreviewerShow(WId wid)
{
    if (m_appPreviewer->isHidden())
    {
        AppButton *button = (AppButton *)sender();
        QPoint center = mapToGlobal(button->geometry().center());
        emit previewerShow(wid, center);
    }
    else
    {
        emit previewerHide(wid);
    }
}

void AppGroup::closeWindow(WId wid)
{
    WindowInfoHelper::closeWindow(wid);
    m_appPreviewer->hide();
}

void AppGroup::init()
{
    auto direction = getLayoutDirection();
    m_layout = new QBoxLayout(direction, this);
    m_layout->setSpacing(1);
    m_layout->setContentsMargins(0, 0, 0, 0);
    setLayout(m_layout);

    QObject *Object = dynamic_cast<QObject *>(m_import->getPanel());
    connect(Object, SIGNAL(panelProfileChanged()), this, SLOT(updateLayout()));

    AppButtonContainer *appButtonContainer = (AppButtonContainer *)parent();
    connect(appButtonContainer, &AppButtonContainer::windowAdded, this, &AppGroup::addWindow);
    connect(appButtonContainer, &AppButtonContainer::windowRemoved, this, &AppGroup::removeWindow);

    connect(appButtonContainer, &AppButtonContainer::windowChanged, this, &AppGroup::windowChanged);
    connect(appButtonContainer, &AppButtonContainer::activeWindowChanged, this, &AppGroup::changedActiveWindow);
    connect(appButtonContainer, &AppButtonContainer::activeWindowChanged, this, &AppGroup::activeWindowChanged);

    m_appPreviewer = new AppPreviewer(m_import, this);
    connect(m_appPreviewer, &AppPreviewer::windowClose, this, &AppGroup::closeWindow);

    // QSettings 保存时，会删除原有文件，重新创建一个新文件，所以不能监视文件，此处监视文件夹
    m_settingFileWatcher.addPath(QFileInfo(KIRAN_SHELL_SETTING_FILE).dir().path());
    connect(&m_settingFileWatcher, &QFileSystemWatcher::directoryChanged, this, [=]()
            { updateLayout(); });

    m_buttonFixed->setAppInfo(m_appBaseInfo);

    updateLayout();
}

void AppGroup::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        m_pressPoint = event->pos();
    }

    QWidget::mousePressEvent(event);
}

void AppGroup::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        if (qAbs(event->pos().x() - m_pressPoint.x()) < 10 && qAbs(event->pos().y() - m_pressPoint.y()) < 10)
        {
            QWidget::mouseMoveEvent(event);
            return;
        }

        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;
        QByteArray ba;
        ba.append(m_appBaseInfo.m_url.toString().toLocal8Bit());
        mimeData->setData("text/uri-list", ba);
        drag->setMimeData(mimeData);
        drag->exec(Qt::MoveAction);
    }

    QWidget::mouseMoveEvent(event);
}

void AppGroup::addWindow(QByteArray wmClass, WId wid)
{
    if (m_mapWidButton.contains(wid))
    {
        return;
    }

    QUrl url = WindowInfoHelper::getUrlByWId(wid);
    if (url != m_appBaseInfo.m_url)
    {
        return;
    }

    AppButton *appButton = newAppBtn();
    m_appBaseInfo.m_wmClass = wmClass;

    appButton->setAppInfo(wmClass, wid);

    m_mapWidButton[wid] = appButton;

    updateLayout();

    emit windowAdded(wmClass, wid);
}

void AppGroup::removeWindow(WId wid)
{
    auto iter = m_mapWidButton.begin();
    while (iter != m_mapWidButton.end())
    {
        if (iter.key() == wid)
        {
            break;
        }
        iter++;
    }

    if (iter == m_mapWidButton.end())
    {
        return;
    }

    AppButton *appButton = iter.value();
    m_mapWidButton.erase(iter);

    delete appButton;
    appButton = nullptr;

    //    if (appButton != m_buttonFixed)
    //    {

    //    }
    //    m_buttonFixed->reset();

    updateLayout();

    // 是否已空
    int size = 0;
    getRelationAppSize(size);
    if (0 == size && !m_appBaseInfo.m_isLocked)
    {
        emit emptyGroup(this);
    }
    else
    {
        emit windowRemoved(wid);
    }
}

void AppGroup::changedActiveWindow(WId wid)
{
    // 激活按钮
    for (auto iter : m_mapWidButton)
    {
        iter->setChecked(false);
    }
    // 锁定按钮关联的应用被关闭后，导致不在m_mapWidButton中，需要单独更新
    if (m_buttonFixed)
    {
        m_buttonFixed->setChecked(false);
    }

    auto iter = m_mapWidButton.begin();
    while (iter != m_mapWidButton.end())
    {
        if (iter.key() == wid)
        {
            break;
        }
        iter++;
    }

    if (iter == m_mapWidButton.end())
    {
        return;
    }

    // 按钮分离显示时，只需要将wid对应的按钮切换状态
    // 否则，需要查找该类的第一个按钮切换状态
    if (SettingProcess::getValue(TASKBAR_SHOW_APP_BTN_TAIL_KEY).toBool())
    {
        iter.value()->setChecked(true);
    }
    else
    {
        m_mapWidButton.first()->setChecked(true);
    }
}

void AppGroup::updateLayout()
{
    Utility::clearLayout(m_layout, false, true);

    //横竖摆放
    auto direction = getLayoutDirection();
    m_layout->setDirection(direction);
    //子控件对齐方式：左右、上下
    Qt::AlignmentFlag alignment = getLayoutAlignment();
    m_layout->setAlignment(alignment);

    QList<AppButton *> appButtons;
    if (m_mapWidButton.isEmpty() && m_appBaseInfo.m_isLocked)
    {
        appButtons.append(m_buttonFixed);
        m_buttonFixed->setShowVisualName(false);
    }
    else
    {
        // 根据当前模式，显示不一样的结果
        if (SettingProcess::getValue(TASKBAR_SHOW_APP_BTN_TAIL_KEY).toBool())
        {
            for (auto iter : m_mapWidButton)
            {
                appButtons.append(iter);
                iter->setShowVisualName(true);
            }
        }
        else
        {
            if (!m_mapWidButton.isEmpty())
            {
                appButtons.append(m_mapWidButton.first());
                m_mapWidButton.first()->setShowVisualName(false);
            }
        }
    }

    for (auto btn : appButtons)
    {
        btn->show();
        m_layout->addWidget(btn);
    }
}

AppButton *AppGroup::newAppBtn()
{
    AppButtonContainer *appButtonContainer = (AppButtonContainer *)parent();
    AppButton *appButton = new AppButton(m_import, this);
    connect(appButton, &AppButton::previewerShowChange, this, &AppGroup::changePreviewerShow);
    connect(appButton, &AppButton::windowClose, this, &AppGroup::closeWindow);
    connect(appButton, &AppButton::isInFavorite, this, &AppGroup::isInFavorite, Qt::DirectConnection);
    connect(appButton, &AppButton::isInTasklist, this, &AppGroup::isInTasklist, Qt::DirectConnection);
    connect(appButton, &AppButton::addToFavorite, this, &AppGroup::addToFavorite);
    connect(appButton, &AppButton::removeFromFavorite, this, &AppGroup::removeFromFavorite);
    connect(appButton, &AppButton::addToTasklist, this, [this](const QUrl url)
            { emit addToTasklist(url, this); });
    connect(appButton, &AppButton::removeFromTasklist, this, &AppGroup::removeFromTasklist);
    connect(appButton, &AppButton::getRelationAppSize, this, &AppGroup::getRelationAppSize, Qt::DirectConnection);

    // 需要显示时才显示
    appButton->hide();

    return appButton;
}

Qt::AlignmentFlag AppGroup::getLayoutAlignment()
{
    int orientation = m_import->getPanel()->getOrientation();
    Qt::AlignmentFlag alignment = (orientation == PanelOrientation::PANEL_ORIENTATION_BOTTOM ||
                                   orientation == PanelOrientation::PANEL_ORIENTATION_TOP)
                                      ? Qt::AlignLeft
                                      : Qt::AlignTop;

    return alignment;
}

QBoxLayout::Direction AppGroup::getLayoutDirection()
{
    int orientation = m_import->getPanel()->getOrientation();
    auto direction = (orientation == PanelOrientation::PANEL_ORIENTATION_BOTTOM ||
                      orientation == PanelOrientation::PANEL_ORIENTATION_TOP)
                         ? QBoxLayout::Direction::LeftToRight
                         : QBoxLayout::Direction::TopToBottom;
    return direction;
}

}  // namespace Taskbar
}  // namespace Kiran
