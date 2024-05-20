#pragma once
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>

class InetAddress
{
private:
    sockaddr_in addr_;
public:
    InetAddress(const std::string &ip,const uint16_t &port);
    InetAddress(const sockaddr_in addr);
    ~InetAddress();
    const char *ip()const;  //返回ip地址字符串
    uint16_t port()const ; //返回端口
    const sockaddr *addr() const; // 返回addr成员的地址，转换成了sockaddr
};


// InetAddress 类：封装了IP地址和端口号。通过ip和端口构造