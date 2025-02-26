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
#include <QGSettings>
#include <QMouseEvent>

#include "app-button.h"
#include "app-group.h"
#include "app-previewer.h"
#include "ks-i.h"
#include "lib/common/logging-category.h"
#include "lib/common/utility.h"
#include "lib/common/window-info-helper.h"
#include "plugin-i.h"
#include "window.h"

namespace Kiran
{
namespace Taskbar
{
AppGroup::AppGroup(IAppletImport *import, const AppInfo &appInfo, QWidget *parent)
    : QWidget(parent),
      m_import(import),
      m_appBaseInfo(appInfo),
      m_buttonFixed(newAppBtn())
{
    init();
}

AppGroup::AppGroup(IAppletImport *import, QWidget *parent)
    : QWidget(parent),
      m_import(import),
      m_buttonFixed(newAppBtn())
{
    m_isLocked = true;

    init();

    m_buttonFixed->show();
}

const AppInfo &AppGroup::getAppBaseInfo()
{
    return m_appBaseInfo;
}

bool AppGroup::isLocked() const
{
    return m_isLocked;
}

void AppGroup::setLocked(bool lockFlag)
{
    m_isLocked = lockFlag;
    updateLayout();
}

void AppGroup::setDragData(const QUrl &url)
{
    if (m_buttonFixed)
    {
        m_buttonFixed->setUrl(url);
        m_buttonFixed->show();
    }
}

bool AppGroup::isOpened() const
{
    return !m_mapWidButton.isEmpty();
}

void AppGroup::getRelationAppSize(int &size)
{
    if (m_gsettings->get(TASKBAR_SCHEMA_KEY_SHOW_APP_NAME).toBool() && !m_mapWidButton.empty())
    {
        size = 1;
    }
    else
    {
        size = m_mapWidButton.size();
    }
}

void AppGroup::showPreviewer(WId wid)
{
    QList<WId> wids;
    if (!m_gsettings->get(TASKBAR_SCHEMA_KEY_SHOW_APP_NAME).toBool())
    {
        wids = m_mapWidButton.keys();
    }
    else
    {
        wids << wid;
    }
    emit previewerShow(wids, (QWidget *)sender());
}

void AppGroup::changePreviewerShow(WId wid)
{
    QList<WId> wids;
    if (!m_gsettings->get(TASKBAR_SCHEMA_KEY_SHOW_APP_NAME).toBool())
    {
        wids = m_mapWidButton.keys();
    }
    else
    {
        wids << wid;
    }

    emit previewerShowChange(wids, (QWidget *)sender());
}

void AppGroup::windowCloseAll()
{
    for (auto wid : m_mapWidButton.keys())
    {
        WindowInfoHelper::closeWindow(wid);
    }
}

void AppGroup::init()
{
    m_gsettings = new QGSettings(TASKBAR_SCHEMA_ID, "", this);

    auto direction = getLayoutDirection();
    m_layout = new QBoxLayout(direction, this);
    m_layout->setSpacing(8);
    m_layout->setContentsMargins(0, 0, 0, 0);

    auto *window = (Window *)parent();
    // connect(window, &Window::windowRemoved, this, &AppGroup::removeWindow);
    connect(window, &Window::windowChanged, this, &AppGroup::windowChanged);
    connect(window, &Window::activeWindowChanged, this, &AppGroup::changedActiveWindow);
    connect(window, &Window::activeWindowChanged, this, &AppGroup::activeWindowChanged);
    connect(this, &AppGroup::previewerShow, window, &Window::previewerShow);
    connect(this, &AppGroup::previewerHide, window, &Window::previewerHide);
    connect(this, &AppGroup::previewerShowChange, window, &Window::previewerShowChange);

    m_buttonFixed->setAppInfo(m_appBaseInfo);

    updateLayout();
}

void AppGroup::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        dragStartPosition = event->globalPos();
        buttonStartPosition = geometry().topLeft();
    }
}

void AppGroup::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        // 防误触
        if (qAbs(event->globalPos().x() - dragStartPosition.x()) < 10 && qAbs(event->globalPos().y() - dragStartPosition.y()) < 10)
        {
            QWidget::mouseMoveEvent(event);
            return;
        }

        // 提升，显示在最前面
        raise();
        emit moveGroupStarted(this);
        emit previewerHide();

        // 鼠标移动偏移量
        QPoint delta = event->globalPos() - dragStartPosition;

        // 移动到新位置
        Qt::AlignmentFlag alignment = getLayoutAlignment();
        if (Qt::AlignLeft == alignment)
        {
            move(buttonStartPosition.x() + delta.x(), buttonStartPosition.y());
        }
        else
        {
            move(buttonStartPosition.x(), buttonStartPosition.y() + delta.y());
        }

        emit groupMoved(this);

        event->accept();
    }
}

void AppGroup::mouseReleaseEvent(QMouseEvent *event)
{
    event->accept();

    emit moveGroupEnded(this);
}

void AppGroup::addWindow(WId wid)
{
    if (m_mapWidButton.contains(wid))
    {
        return;
    }

    AppButton *appButton = newAppBtn();
    appButton->setAppInfo(m_appBaseInfo, wid);

    m_mapWidButton[wid] = appButton;

    updateLayout();
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

    updateLayout();

    // 是否已空
    int size = 0;
    getRelationAppSize(size);
    if (0 == size && !m_isLocked)
    {
        emit emptyGroup(this);
    }
}

void AppGroup::changedActiveWindow(WId wid)
{
    // 激活按钮
    for (auto *iter : m_mapWidButton)
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
    if (m_gsettings->get(TASKBAR_SCHEMA_KEY_SHOW_APP_NAME).toBool())
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

    // 横竖摆放
    auto direction = getLayoutDirection();
    m_layout->setDirection(direction);
    // 子控件对齐方式：左右、上下
    Qt::AlignmentFlag alignment = getLayoutAlignment();
    m_layout->setAlignment(alignment);

    QList<AppButton *> appButtons;
    if (m_mapWidButton.isEmpty() && m_isLocked)
    {
        appButtons.append(m_buttonFixed);
        m_buttonFixed->setShowVisualName(false);
    }
    else
    {
        // 根据当前模式，显示不一样的结果
        if (m_gsettings->get(TASKBAR_SCHEMA_KEY_SHOW_APP_NAME).toBool())
        {
            for (auto *iter : m_mapWidButton)
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

    for (auto *btn : appButtons)
    {
        btn->show();
        m_layout->addWidget(btn);
    }
}

AppButton *AppGroup::newAppBtn()
{
    auto *appButton = new AppButton(m_import, this);
    connect(appButton, &AppButton::previewerShow, this, &AppGroup::showPreviewer);
    connect(appButton, &AppButton::previewerHide, this, &AppGroup::previewerHide);
    connect(appButton, &AppButton::previewerShowChange, this, &AppGroup::changePreviewerShow);
    connect(appButton, &AppButton::windowCloseAll, this, &AppGroup::windowCloseAll);

    connect(appButton, &AppButton::isInFavorite, this, &AppGroup::isInFavorite, Qt::DirectConnection);
    connect(appButton, &AppButton::isInTasklist, this, &AppGroup::isInTasklist, Qt::DirectConnection);
    connect(appButton, &AppButton::addToFavorite, this, &AppGroup::addToFavorite);
    connect(appButton, &AppButton::removeFromFavorite, this, &AppGroup::removeFromFavorite);
    connect(appButton, &AppButton::addToTasklist, this, [this](const QUrl &url)
            {
                emit addToTasklist(url, this);
            });
    connect(appButton, &AppButton::removeFromTasklist, this, &AppGroup::removeFromTasklist);
    connect(appButton, &AppButton::getRelationAppSize, this, &AppGroup::getRelationAppSize, Qt::DirectConnection);

    // 点击反向穿透，用于支持拖拽
    // 当子控件能点击时，父控件无法收到点击事件
    connect(appButton, &AppButton::mousePressed, this, &AppGroup::mousePressEvent);
    connect(appButton, &AppButton::mouseMoved, this, &AppGroup::mouseMoveEvent);
    connect(appButton, &AppButton::mouseReleased, this, &AppGroup::mouseReleaseEvent);

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
