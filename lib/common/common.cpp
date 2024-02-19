#include <qt5-log-i.h>
#include <QProcess>
#include <QSettings>
#include <QWidget>

#include "common.h"
#include "define.h"

QByteArray runCmd(QString cmd, QStringList cmdArg)
{
    //    KLOG_INFO() << "runCmd" << cmd << cmdArg;
    QProcess p(0);
    p.start(cmd, cmdArg);
    p.waitForStarted();
    p.waitForFinished();
    return p.readAll();
}

void clearLayout(QLayout *layout)
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

QString getElidedText(QFontMetrics fontMetrics, QString text, int elidedTextLen)
{
    return fontMetrics.elidedText(text, Qt::ElideRight, elidedTextLen);
}

bool isShowAppBtnTail()
{
    QSettings kiranShellSetting(KIRAN_SHELL_SETTING_FILE, QSettings::IniFormat);
    bool isShow = kiranShellSetting.value(TASKBAR_SHOW_APP_BTN_TAIL_KEY).toBool();
    return isShow;
}

void saveIsShowAppBtnTail(bool isShow)
{
    QString file = KIRAN_SHELL_SETTING_FILE;

    KLOG_INFO() << "saveIsShowAppBtnTail" << isShow << file;
    QSettings kiranShellSetting(KIRAN_SHELL_SETTING_FILE, QSettings::IniFormat);
    kiranShellSetting.setValue(TASKBAR_SHOW_APP_BTN_TAIL_KEY, isShow);
}
