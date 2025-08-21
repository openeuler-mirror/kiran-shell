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
#include <QString>
#include <QStringList>

class QGSettings;
namespace Kiran
{
namespace Systemtray
{
class TraySettings
{
public:
    TraySettings();
    ~TraySettings();

    bool isFoldingApp(const QString& id) const;

    QStringList getFoldingApps() const;
    void setFoldingApps(const QStringList &ids);

    void addFoldingApp(const QString& id);
    void removeFoldingApp(const QString &id);
    
private:
    QGSettings* m_settings;
};

} // namespace Systemtray
} // namespace Kiran
