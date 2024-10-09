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

#include <QObject>

#include "status-notifier-watcher.h"

class TrayServer : public QObject
{
    Q_OBJECT
public:
    static TrayServer& Instance()
    {
        static TrayServer instance;
        return instance;
    }

    TrayServer(const TrayServer&) = delete;
    TrayServer(TrayServer&&) = delete;
    TrayServer& operator=(const TrayServer&) = delete;
    TrayServer& operator=(TrayServer&&) = delete;

private:
    TrayServer();
    ~TrayServer();

private:
    // 注册用
    StatusNotifierWatcher* m_statusNotifierWatcher;
};

#define TrayServerInstance TrayServer::Instance()
