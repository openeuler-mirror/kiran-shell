/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd.
 * kiran-shell is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#include <qt5-log-i.h>
#include <QApplication>
#include <QCommandLineParser>
#include <QFileInfo>
#include <QProcess>
#include <QTranslator>

#include "ks-config.h"
#include "lib/common/logging-category.h"
#include "profile/profile.h"
#include "shell.h"

// using namespace Kiran;

int main(int argc, char *argv[])
{
    auto argv0 = QFileInfo(argv[0]);
    auto programName = argv0.baseName();

    QApplication app(argc, argv);

    qApp->setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication::setApplicationName(programName);
    QApplication::setApplicationVersion(PROJECT_VERSION);

#ifndef QT_DEBUG
    if (klog_qt5_init("", "kylinsec-session", "kiran-shell", "kiran-shell") != 0)
    {
        fprintf(stderr, "Failed to init kiran-log.");
    }
#endif

    auto local = QLocale();
    KLOG_INFO(LCShell) << "current local:" << local << local.name();
    QTranslator translator;
    if (!translator.load(QLocale(), "kiran-shell", ".", KS_INSTALL_TRANSLATIONDIR, ".qm"))
    {
        KLOG_WARNING(LCShell) << "Load translator failed!";
    }
    else
    {
        QApplication::installTranslator(&translator);
    }

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption resetOption(QStringList() << "reset", QObject::tr("Reset the panel ."));
    parser.addOption(resetOption);

    parser.process(app);

    if (parser.isSet(resetOption))
    {
        QProcess::startDetached("gsettings", {"reset-recursively", "com.kylinsec.kiran.shell"});
        return 0;
    }

    Kiran::Profile::globalInit();
    Kiran::Shell::globalInit();

    auto ret = QApplication::exec();

    Kiran::Shell::globalDeinit();
    Kiran::Profile::globalDeinit();

    return ret;
}
