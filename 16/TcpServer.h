#pragma once
#include <iostream>
#include "EventLoop.h"
#include "Socket.h"
#include "Channel.h"
#include "Acceptor.h"
#include <map>

#include "Connection.h"
using namespace std;
class TcpServer
{
private:
    EventLoop loop_; //一个TcpSErver可以有多个事件循环
    Acceptor *acceptor_; //一个TcpServer只有一个Accept类
    std::map<int,Connection*> conns_;
    // 一个TcpServer有多个Connection对象，存放在map容器中。
public:
    TcpServer(const std::string &ip,const uint16_t port);
    ~TcpServer();

    void start();

    void newconnection(Socket *clientsock);    // 处理新客户端连接请求。
    void closeconnection(Connection *conn);  // 关闭客户端的连接，在Connection类中回调此函数。 
    void errorconnection(Connection *conn);  // 客户端的连接错误，在Connection类中回调此函数。
};