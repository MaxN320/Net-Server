#include "InetAddress.h"

InetAddress::InetAddress()
{

}

InetAddress::InetAddress(const std::string &ip,const uint16_t& port)
{
    addr_.sin_family=AF_INET;
    addr_.sin_addr.s_addr=inet_addr(ip.c_str());
    addr_.sin_port=htons(port);
}
InetAddress::InetAddress(const sockaddr_in addr):addr_(addr)
{
    
}

InetAddress::~InetAddress()
{

}


const char *InetAddress::ip() const                // 返回字符串表示的地址，例如：192.168.150.128
{
    return inet_ntoa(addr_.sin_addr);
}

uint16_t InetAddress::port() const                // 返回整数表示的端口，例如：80、8080
{
    return ntohs(addr_.sin_port);
}

const sockaddr *InetAddress::addr() const   // 返回addr_成员的地址，转换成了sockaddr。
{
    return (sockaddr*)&addr_;
}
void  InetAddress::setaddr(sockaddr_in clientaddr)
{
    addr_=clientaddr;
}