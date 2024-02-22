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
    QSettings kiranShellSetting(KIRAN_SHELL_SETTING_FILE, QSettings::IniFormat);
    kiranShellSetting.setValue(TASKBAR_SHOW_APP_BTN_TAIL_KEY, isShow);
}

QStringList getTaskBarLockApp()
{
    QSettings kiranShellSetting(KIRAN_SHELL_SETTING_FILE, QSettings::IniFormat);
    QStringList apps = kiranShellSetting.value(TASKBAR_LOCK_APP_KEY).toStringList();
    return apps;
}

void addTaskBarLockApp(QString appId)
{
    QSettings kiranShellSetting(KIRAN_SHELL_SETTING_FILE, QSettings::IniFormat);
    QStringList apps = kiranShellSetting.value(TASKBAR_LOCK_APP_KEY).toStringList();
    if (!apps.contains(appId))
    {
        apps.append(appId);
        kiranShellSetting.setValue(TASKBAR_LOCK_APP_KEY, apps);
    }
}

void removeTaskBarLockApp(QString appId)
{
    QSettings kiranShellSetting(KIRAN_SHELL_SETTING_FILE, QSettings::IniFormat);
    QStringList apps = kiranShellSetting.value(TASKBAR_LOCK_APP_KEY).toStringList();
    if (apps.contains(appId))
    {
        apps.removeAll(appId);
        kiranShellSetting.setValue(TASKBAR_LOCK_APP_KEY, apps);
    }
}

bool isTaskBarLockApp(QString appId)
{
    QSettings kiranShellSetting(KIRAN_SHELL_SETTING_FILE, QSettings::IniFormat);
    QStringList apps = kiranShellSetting.value(TASKBAR_LOCK_APP_KEY).toStringList();
    if (apps.contains(appId))
    {
        return true;
    }

    return false;
}

