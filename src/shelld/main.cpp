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

#include <QCoreApplication>
#include <memory>

#include "shelld.h"

// TODO:当前只有底部面板用到,后面如果功能多了,或者其他情况,考虑移动到控制中心后端
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    Kiran::Shelld shelld;

    return QCoreApplication::exec();  // keep running
}
