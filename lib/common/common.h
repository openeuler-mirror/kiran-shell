#ifndef COMMON_H
#define COMMON_H

#include <QFontMetrics>
#include <QLayout>
#include <QString>
#include <QStringList>

// 执行命令
QByteArray runCmd(QString cmd, QStringList cmdArg = QStringList());

// 清理布局
void clearLayout(QLayout *layout);

// 获取含省略号的字符串
QString getElidedText(QFontMetrics fontMetrics, QString text, int elidedTextLen);

// 从配置文件中 获取/设置 是否显示按钮后缀信息（任务栏按钮显示软件名称）
bool isShowAppBtnTail();
void saveIsShowAppBtnTail(bool isShow);

#endif  // COMMON_H
