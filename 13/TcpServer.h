#pragma once
#include <iostream>
#include "EventLoop.h"
#include "Socket.h"
#include "Channel.h"
#include "Acceptor.h"


#include "Connection.h"
using namespace std;
class TcpServer
{
private:
    EventLoop loop_; //一个TcpSErver可以有多个事件循环
    Acceptor *acceptor_; //一个TcpServer只有一个Accept类
public:
    TcpServer(const std::string &ip,const uint16_t port);
    ~TcpServer();

    void start();

    void newconnection(Socket *clientsock);    // 处理新客户端连接请求。
};