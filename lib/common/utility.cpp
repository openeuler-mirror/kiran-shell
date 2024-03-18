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

#include <QProcess>
#include <QWidget>

#include "utility.h"

QByteArray Utility::runCmd(QString cmd, QStringList cmdArg)
{
    // KLOG_INFO() << "runCmd" << cmd << cmdArg;
    QProcess p(0);
    p.start(cmd, cmdArg);
    p.waitForStarted();
    p.waitForFinished();
    return p.readAll();
}

void Utility::clearLayout(QLayout *layout)
{
    QLayoutItem *child;
    while ((child = layout->takeAt(0)) != 0)
    {
        if (child->widget())
        {
            child->widget()->setParent(NULL);
        }
        delete child;
    }
}

QString Utility::getElidedText(QFontMetrics fontMetrics, QString text, int elidedTextLen)
{
    return fontMetrics.elidedText(text, Qt::ElideRight, elidedTextLen);
}
