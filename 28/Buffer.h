#pragma once
#include <string>
#include <iostream>

class Buffer
{
private:
    std::string buf_;
public:
    Buffer();
    ~Buffer();

    void append(const char *data,size_t size);
    void appendwithhead(const char *data,size_t size);
    //把数据追加到buf中,附加报文头部
    void erase(size_t pos,size_t nn);   
    size_t size();
    const char*data();
    void clear();
};