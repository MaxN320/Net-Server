#pragma once

#include <iostream>
#include <string>

class Timestamp
{
private:
    time_t secsinceepoch_;  //整数表示的事件（从1970年开始到现在的秒数）
public:
    Timestamp();            //用当前事件初始化对象
    Timestamp(int64_t secsinceepoch); //用整数事件初始化对象
    static Timestamp now();           //返回当前事件的Timestamp对象
    std::string tostring() const;     //返回字符串表示的时间
    time_t toint() const;
    // 格式 yyyy-mm-dd hh24:mi:ss
};