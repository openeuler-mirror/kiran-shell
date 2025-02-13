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

#include <pinyin.h>
#include <qt5-log-i.h>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QGuiApplication>
#include <QLayout>
#include <QProcess>
#include <QScreen>
#include <QWidget>

#include "ks-config.h"
#include "ks-i.h"
#include "lib/common/logging-category.h"
#include "utility.h"

static pinyin_context_t *pinyinContext = pinyin_init(LIBPINYIN_PKGDATADIR, KS_INSTALL_DATADIR);
static pinyin_instance_t *pinyininstance = pinyinContext ? pinyin_alloc_instance(pinyinContext) : nullptr;

QByteArray Utility::runCmd(QString cmd, QStringList cmdArg)
{
    KLOG_INFO(LCLib) << "run cmd" << cmd << cmdArg;
    QProcess p(0);
    p.start(cmd, cmdArg);
    p.waitForStarted();
    p.waitForFinished();
    return p.readAll();
}

void Utility::clearLayout(QLayout *layout, bool deleteWidget, bool hideWidget)
{
    QLayoutItem *child;
    while ((child = layout->takeAt(0)) != 0)
    {
        QWidget *widget = child->widget();
        if (widget)
        {
            if (deleteWidget)
            {
                // 不再需要子项
                widget->setParent(NULL);
                delete widget;
            }
            else if (hideWidget)
            {
                // 隐藏功能，用于只需要显示部分子项，调用后再次显示需要调用show
                // setParent(NULL)会导致不显示的子项无法自动析构
                layout->removeWidget(widget);
                widget->hide();
            }
            else
            {
                // 若为全量显示，则不需要隐藏
                layout->removeWidget(widget);
            }
        }
        delete child;
    }
}

QString Utility::getElidedText(QFontMetrics fontMetrics, QString text, int elidedTextLen)
{
    return fontMetrics.elidedText(text, Qt::ElideRight, elidedTextLen);
}

void Utility::updatePopWidgetPos(int panelOriention, QWidget *triggerWidget, QWidget *popWidget)
{
    auto baseGeometry = triggerWidget->geometry();
    auto baseCenter = baseGeometry.center();
    auto windowSize = popWidget->frameSize();
    QPoint windowPosition(0, 0);

    // 获取当前屏幕坐标
    QRect screenGeometry = QGuiApplication::primaryScreen()->geometry();
    switch (panelOriention)
    {
    case Kiran::PanelOrientation::PANEL_ORIENTATION_TOP:
        // 以触发窗口下沿中心为弹窗上沿中心
        windowPosition = triggerWidget->mapToGlobal(QPoint(triggerWidget->width() / 2, triggerWidget->height()));
        windowPosition.setX(windowPosition.x() - windowSize.width() / 2);
        // 若超出屏幕，则将窗口定位到屏幕左侧
        if (windowPosition.x() + windowSize.width() > screenGeometry.width())
        {
            windowPosition.setX(screenGeometry.width() - windowSize.width());
        }
        break;
    case Kiran::PanelOrientation::PANEL_ORIENTATION_RIGHT:
        // 以触发窗口左沿中心为弹窗右沿中心
        windowPosition = triggerWidget->mapToGlobal(QPoint(0, triggerWidget->height() / 2));
        windowPosition.setX(windowPosition.x() - windowSize.width());
        windowPosition.setY(windowPosition.y() - windowSize.height() / 2);
        // 若超出屏幕，则将窗口定位到屏幕底部
        if (windowPosition.y() + windowSize.height() > screenGeometry.height())
        {
            windowPosition.setY(screenGeometry.height() - windowSize.height());
        }
        break;
    case Kiran::PanelOrientation::PANEL_ORIENTATION_BOTTOM:
        // 以触发窗口上沿中心为弹窗下沿中心
        windowPosition = triggerWidget->mapToGlobal(QPoint(triggerWidget->width() / 2, 0));
        windowPosition.setX(windowPosition.x() - windowSize.width() / 2);
        windowPosition.setY(windowPosition.y() - windowSize.height());
        if (windowPosition.x() + windowSize.width() > screenGeometry.width())
        {
            windowPosition.setX(screenGeometry.width() - windowSize.width());
        }
        break;
    case Kiran::PanelOrientation::PANEL_ORIENTATION_LEFT:
        // 以触发窗口右沿中心为弹窗左沿中心
        windowPosition = triggerWidget->mapToGlobal(QPoint(triggerWidget->width(), triggerWidget->height() / 2));
        windowPosition.setY(windowPosition.y() - windowSize.height() / 2);
        if (windowPosition.y() + windowSize.height() > screenGeometry.height())
        {
            windowPosition.setY(screenGeometry.height() - windowSize.height());
        }
        break;
    default:
        KLOG_WARNING(LCLib) << "Unknown oriention " << panelOriention;
        break;
    }

    popWidget->move(windowPosition);
}

bool Utility::isDbusServiceRegistered(QString serviceName, QDBusConnection::BusType type)
{
    QDBusConnectionInterface *connectionInterface = nullptr;
    switch (type)
    {
    case QDBusConnection::SessionBus:
    {
        connectionInterface = QDBusConnection::sessionBus().interface();
        break;
    }
    case QDBusConnection::SystemBus:
    {
        connectionInterface = QDBusConnection::systemBus().interface();
        break;
    }
    default:
        break;
    }

    if (connectionInterface && connectionInterface->isServiceRegistered(serviceName))
    {
        KLOG_INFO(LCLib) << "Service is available:" << serviceName;
        return true;
    }
    else
    {
        KLOG_WARNING(LCLib) << "Service is not available:" << serviceName;
        return false;
    }
}

QStringList Utility::pinyinGuess(const QString &pinyinInput)
{
    QStringList resultList;

    // 拼音上下文
    if (!pinyinContext)
    {
        KLOG_ERROR(LCLib) << "Failed to init pinyin context.";
        return resultList;
    }

    // 拼音实例
    if (!pinyininstance)
    {
        KLOG_ERROR(LCLib) << "Failed to alloc pinyin instance.";
        return resultList;
    }

    // 设置输入模式
    pinyin_option_t options = PINYIN_INCOMPLETE | PINYIN_CORRECT_ALL | USE_DIVIDED_TABLE | USE_RESPLIT_TABLE | DYNAMIC_ADJUST;
    pinyin_set_options(pinyinContext, options);

    // 添加拼音输入
    size_t len = pinyin_parse_more_full_pinyins(pinyininstance, pinyinInput.toUtf8().constData());
    if (len == 0)
    {
        KLOG_WARNING(LCLib) << "Failed to parse pinyin input.";
        // pinyin_free_instance(pinyininstance);
        // pinyin_fini(pinyinContext);
        return resultList;
    }

    // 猜测候选
    pinyin_guess_candidates(pinyininstance, 0, SORT_BY_PHRASE_LENGTH | SORT_BY_FREQUENCY);

    // 获取候选
    unsigned int num = 0;
    pinyin_get_n_candidate(pinyininstance, &num);
    for (unsigned int i = 0; i < num; ++i)
    {
        lookup_candidate_t *candidate = NULL;
        pinyin_get_candidate(pinyininstance, i, &candidate);

        const char *word = NULL;
        pinyin_get_candidate_string(pinyininstance, candidate, &word);

        resultList.append(QString::fromUtf8(word));
    }

    pinyin_reset(pinyininstance);

    // 清理资源
    // pinyin_free_instance(pinyininstance);
    // pinyin_fini(pinyinContext);

    return resultList.mid(0, 20);
}

Utility::Utility()
{
}

Utility::~Utility()
{
    if (pinyininstance)
    {
        pinyin_free_instance(pinyininstance);
    }
    if (pinyinContext)
    {
        pinyin_fini(pinyinContext);
    }
}
