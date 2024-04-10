/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd. 
 * kiran-session-manager is licensed under Mulan PSL v2.
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
#include <QTranslator>
#include "ks-config.h"
#include "profile/profile.h"
#include "shell.h"

// using namespace Kiran;

int main(int argc, char *argv[])
{
    auto argv0 = QFileInfo(argv[0]);
    auto programName = argv0.baseName();

    //    if (klog_qt5_init(QString(), "kylinsec-session", PROJECT_NAME, programName) < 0)
    //    {
    //        fprintf(stderr, "Failed to init kiran-log.");
    //    }

    qApp->setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication app(argc, argv);
    app.setApplicationName(programName);
    app.setApplicationVersion(PROJECT_VERSION);

    auto local = QLocale();
    KLOG_INFO() << "current local:" << local << local.name();
    QTranslator translator;
    if (!translator.load("/usr/local/share/kiran-shell/translations/kiran-shell.zh_CN.qm"))
    //    if (!translator.load(QLocale(), "kiran-shell", ".", KS_INSTALL_TRANSLATIONDIR, ".qm"))
    {
        KLOG_WARNING() << "Load translator failed!";
    }
    else
    {
        app.installTranslator(&translator);
    }

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);

    Kiran::Profile::globalInit();
    Kiran::Shell::globalInit();

    auto ret = app.exec();

    Kiran::Shell::globalDeinit();
    Kiran::Profile::globalDeinit();

    return ret;
}
