
#pragma once

#include <QObject>

class Lunar : public QObject
{
    Q_OBJECT
public:
    //农历日 字符串 闰三月/三月/初三/植树节
    static QString getLunarDayStr(int year, int month, int day);
    //农历月日 字符串 闰三月初三
    static QString getLunarMonDayStr(int year, int month, int day);
    //农历年 字符串 甲辰龙年
    static QString getLunarYearStr(int year);

private:
    Lunar(){};

    //计算当前日期距离春节的天数
    static int daysSinceSpringFestival(int year, int month, int day, int &yearCountData);
    //判断是否是闰年
    static bool isLoopYear(int year);

    //公历假日
    static QString holiday(int month, int day);
    //计算24节气
    static QString solarTerms(int year, int month, int day);
    //农历节日
    static QString lunarFestival(int year, int month, int day);
    //计算转换的农历日期
    static bool findLunarData(int &year, int &month, int &day);
};
