#pragma once
#include <sys/types.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "InetAddress.h"

//创建一个非阻塞的Socket
int createnonblocking();

class Socket
{
private:
    const int fd_;    //Socket持有的fd，从构造函数传来
    std::string ip_;
    uint16_t port_;
public:
    Socket(int fd);    //构造函数
    ~Socket();

    int fd()const ;  //返回fd_成员
    void setreuseaddr(bool on); //设置SO_REUSEADDR选项，ture打开，false关闭
    void setreuseport(bool on); //设置SO_REUSEPORT
    void settcpnodelay(bool on); //设置SO_NODELAY
    void setkeepalive(bool on);  //设置SO_KEEPALIVE

    void bind(const InetAddress& servaddr); //服务端的socket将调用此函数
    void listen(int nn=128);                
    int accept(InetAddress& clientaddr);   

    std::string ip() const;
    uint16_t port()const;

};
