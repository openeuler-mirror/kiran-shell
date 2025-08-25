/**
 * Copyright (c) 2025 ~ 2026 KylinSec Co., Ltd.
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

#include <QLabel>

class QResizeEvent;
class ElidedLabel : public QLabel
{
    Q_OBJECT
public:
    ElidedLabel(QWidget *parent = nullptr);
    ~ElidedLabel() = default;

    void setShowText(QString text);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void showText();

private:
    QString m_text;
};
