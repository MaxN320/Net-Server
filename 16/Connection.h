#pragma once
#include <functional>
#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"
#include "EventLoop.h"

class Connection
{
private:
    EventLoop *loop_;
    Socket * clientsock_;
    Channel * clientchannel_;
    std::function<void(Connection*)> closecallback_;                   // 关闭fd_的回调函数，将回调TcpServer::closeconnection()。
    std::function<void(Connection*)> errorcallback_;                   // fd_发生了错误的回调函数，将回调TcpServer::errorconnection()。

public:
    Connection(EventLoop* loop,Socket* clientsock);
    ~Connection();

    int fd()const;
    std::string ip()const;
    uint16_t port()const;

    void closecallback();
    void errorcallback();
    void setclosecallback(std::function<void(Connection*)> fn);    // 设置关闭fd_的回调函数。
    void seterrorcallback(std::function<void(Connection*)> fn);    // 设置fd_发生了错误的回调函数。

};