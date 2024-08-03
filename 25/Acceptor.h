#pragma once
#include <functional>
#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"
#include "EventLoop.h"

class Acceptor
{
private:
    EventLoop *loop_;
    Socket *servsock_;
    Channel *acceptchannel_;
    std::function<void(Socket*)> newconnectioncb_; //处理新客户端连接请求的回调函数，将指向TcpServer::newconnection
public:
    Acceptor(EventLoop* loop,const std::string &ip,const uint16_t port);
    ~Acceptor();

    void newconnection(); //处理都事件

    void setnewconnectioncb(std::function<void(Socket*)> fn);
};

