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
#pragma once

#include <QBoxLayout>
#include <QWidget>

#include "plugin-i.h"

class KiranColorBlock;
namespace Kiran
{
class IAppletImport;

class Spacer : public QWidget
{
    Q_OBJECT
public:
    explicit Spacer(IAppletImport *import);

private slots:
    // panel布局信息发生变化
    void updateLayout();

private:
    Qt::AlignmentFlag getLayoutAlignment();
    QBoxLayout::Direction getLayoutDirection();

private:
    IAppletImport *m_import;

    QBoxLayout *m_layout;
    KiranColorBlock *m_prevBlock;
    KiranColorBlock *m_nextBlock;
};

class SpacerPlugin : public QObject, public IPlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID IAPPLET_IID FILE "spacer.json")
    Q_INTERFACES(Kiran::IPlugin)

public:
    QWidget *createApplet(const QString &appletID, IAppletImport *import) override
    {
        return new Spacer(import);
    }
};
}  // namespace Kiran
