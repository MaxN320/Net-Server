#pragma once
#include <functional>
#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Buffer.h"
class Connection
{
private:
    EventLoop *loop_;
    Socket * clientsock_;
    Channel * clientchannel_;
    std::function<void(Connection*)> closecallback_;                   // 关闭fd_的回调函数，将回调TcpServer::closeconnection()。
    std::function<void(Connection*)> errorcallback_;                   // fd_发生了错误的回调函数，将回调TcpServer::errorconnection()。
    std::function<void(Connection*,std::string&)> onmessagecallback_; 
    std::function<void(Connection*)> sendcompletecallback_;
    
    Buffer inputbuffer_;
    Buffer outputbuffer_;
public:
    Connection(EventLoop* loop,Socket* clientsock);
    ~Connection();

    int fd()const;
    std::string ip()const;
    uint16_t port()const;
    void onmessage();                      // 处理对端发送过来的消息。
    void closecallback();
    void errorcallback();
    void writecallback();

    void setonmessagecallback(std::function<void(Connection*,std::string&)> fn);
    void setclosecallback(std::function<void(Connection*)> fn);    // 设置关闭fd_的回调函数。
    void seterrorcallback(std::function<void(Connection*)> fn);    // 设置fd_发生了错误的回调函数。
    void setsendcompletecallback(std::function<void(Connection*)>fn);//发送数据完成后的回调函数

    void send(const char *data,size_t size);
};