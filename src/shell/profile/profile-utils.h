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
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#pragma once

namespace Kiran
{
// 如果后续需要全局通用，再更换定义的位置，目前应该只有这个目录会涉及gsettings

// 函数声明
#define GSETTINGS_PROPERTY_INT_DECLARATION(name, humpName) \
Q_SIGNALS:                                                 \
    void name##Changed(int value);                         \
                                                           \
public:                                                    \
    int get##humpName();                                   \
    void set##humpName(int value);                         \
                                                           \
private:                                                   \
    int m_##name;

#define GSETTINGS_PROPERTY_BOOLEAN_DECLARATION(name, humpName) \
Q_SIGNALS:                                                     \
    void name##Changed(bool value);                            \
                                                               \
public:                                                        \
    bool get##humpName();                                      \
    void set##humpName(bool value);                            \
                                                               \
private:                                                       \
    bool m_##name;

#define GSETTINGS_PROPERTY_STRING_DECLARATION(name, humpName) \
Q_SIGNALS:                                                    \
    void name##Changed(const QString& value);                 \
                                                              \
public:                                                       \
    QString get##humpName();                                  \
    void set##humpName(const QString& value);                 \
                                                              \
private:                                                      \
    QString m_##name;

#define GSETTINGS_PROPERTY_STRINGLIST_DECLARATION(name, humpName) \
Q_SIGNALS:                                                        \
    void name##Changed(const QStringList& value);                 \
                                                                  \
public:                                                           \
    QStringList get##humpName();                                  \
    void set##humpName(const QStringList& value);                 \
                                                                  \
private:                                                          \
    QStringList m_##name;

// 函数定义
#define GSETTINGS_PROPERTY_INT_DEFINITION(className, name, humpName, key) \
                                                                          \
    int className::get##humpName()                                        \
    {                                                                     \
        return this->m_##name;                                            \
    }                                                                     \
    void className::set##humpName(int value)                              \
    {                                                                     \
        if (value != this->m_##name)                                      \
        {                                                                 \
            this->m_##name = value;                                       \
            Q_EMIT this->name##Changed(value);                            \
        }                                                                 \
                                                                          \
        if (value != this->m_settings->get(key).toInt())                  \
        {                                                                 \
            this->m_settings->set(key, QVariant::fromValue(value));       \
        }                                                                 \
    }

#define GSETTINGS_PROPERTY_BOOLEAN_DEFINITION(className, name, humpName, key) \
                                                                              \
    bool className::get##humpName()                                           \
    {                                                                         \
        return this->m_##name;                                                \
    }                                                                         \
    void className::set##humpName(bool value)                                 \
    {                                                                         \
        if (value != this->m_##name)                                          \
        {                                                                     \
            this->m_##name = value;                                           \
            Q_EMIT this->name##Changed(value);                                \
        }                                                                     \
                                                                              \
        if (value != this->m_settings->get(key).toBool())                     \
        {                                                                     \
            this->m_settings->set(key, QVariant::fromValue(value));           \
        }                                                                     \
    }

#define GSETTINGS_PROPERTY_STRING_DEFINITION(className, name, humpName, key) \
                                                                             \
    QString className::get##humpName()                                       \
    {                                                                        \
        return this->m_##name;                                               \
    }                                                                        \
    void className::set##humpName(const QString& value)                      \
    {                                                                        \
        if (value != this->m_##name)                                         \
        {                                                                    \
            this->m_##name = value;                                          \
            Q_EMIT this->name##Changed(value);                               \
        }                                                                    \
                                                                             \
        if (value != this->m_settings->get(key).toString())                  \
        {                                                                    \
            this->m_settings->set(key, QVariant::fromValue(value));          \
        }                                                                    \
    }

#define GSETTINGS_PROPERTY_STRINGLIST_DEFINITION(className, name, humpName, key) \
                                                                                 \
    QStringList className::get##humpName()                                       \
    {                                                                            \
        return this->m_##name;                                                   \
    }                                                                            \
    void className::set##humpName(const QStringList& value)                      \
    {                                                                            \
        if (value != this->m_##name)                                             \
        {                                                                        \
            this->m_##name = value;                                              \
            Q_EMIT this->name##Changed(value);                                   \
        }                                                                        \
                                                                                 \
        if (value != this->m_settings->get(key).toStringList())                  \
        {                                                                        \
            this->m_settings->set(key, QVariant::fromValue(value));              \
        }                                                                        \
    }

// gsettings属性变化case处理
#define GSETTINGS_CASE_INT_CHANGE(caseName, humpName)            \
    case CONNECTION(caseName, _hash):                            \
        this->set##humpName(this->m_settings->get(key).toInt()); \
        break;

#define GSETTINGS_CASE_BOOLEAN_CHANGE(caseName, humpName)         \
    case CONNECTION(caseName, _hash):                             \
        this->set##humpName(this->m_settings->get(key).toBool()); \
        break;

#define GSETTINGS_CASE_STRING_CHANGE(caseName, humpName)            \
    case CONNECTION(caseName, _hash):                               \
        this->set##humpName(this->m_settings->get(key).toString()); \
        break;

#define GSETTINGS_CASE_STRINGLIST_CHANGE(caseName, humpName)            \
    case CONNECTION(caseName, _hash):                                   \
        this->set##humpName(this->m_settings->get(key).toStringList()); \
        break;

#define GSETTINGS_CASE_DEFAULT \
    default:                   \
        break;

}  // namespace Kiran
