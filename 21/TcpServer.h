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

    function<void(Connection*)> newconnectioncb_;
    function<void(Connection*)> closeconnectioncb_;
    function<void(Connection*)> errorconnectioncb_;
    function<void(Connection*,string&message)> onmessagecb_;
    function<void(Connection*)>sendcompletecb_; 
    function<void(EventLoop*)> timeoutcb_;


public:
    TcpServer(const std::string &ip,const uint16_t port);
    ~TcpServer();

    void start();

    void newconnection(Socket *clientsock);    // 处理新客户端连接请求。
    void closeconnection(Connection *conn);  // 关闭客户端的连接，在Connection类中回调此函数。 
    void errorconnection(Connection *conn);  // 客户端的连接错误，在Connection类中回调此函数。
    void onmessage(Connection*conn,std::string message);

    void sendcomplete(Connection *conn); //发送完成后，Connection中回调此函数
    void epolltimeout(EventLoop * loop); //epollwait超时，在Eventloop中回调此函数


    void setnewconnectioncb(std::function<void(Connection*)> fn);
    void setcloseconnectioncb(std::function<void(Connection*)> fn);
    void seterrorconnectioncb(std::function<void(Connection*)> fn);
    void setonmessagecb(std::function<void(Connection*,std::string &message)> fn);
    void setsendcompletecb(std::function<void(Connection*)> fn);
    void settimeoutcb(std::function<void(EventLoop*)> fn);
};