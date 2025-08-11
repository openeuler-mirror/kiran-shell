/**
 * Copyright (c) 2020 ~ 2025 KylinSec Co., Ltd.
 * kiran-shell is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     liuxinhao <liuxinhao@kylinsec.com.cn>
 */
#pragma once
#include <QObject>

class QGSettings;
namespace Kiran
{
namespace Menu
{
class NewAppsManager : public QObject
{
    Q_OBJECT
public:
    static NewAppsManager& getInstance();
    ~NewAppsManager();

    QSet<QString> getAllNewApps() const;
    void markAppsAsRead(const QSet<QString>& appIds);
    void updateNewApps(const QSet<QString>& newAppIds);

signals:
    // 外部更改新应用配置或新应用被打开后触发
    void newAppsConfigUpdated();

private:
    NewAppsManager(QObject* parent = nullptr);
    QGSettings* m_gsettings = nullptr;
};
}  // namespace Menu
}  // namespace Kiran