#pragma once
#include <sys/epoll.h>
#include <functional>
#include "InetAddress.h"
#include "EventLoop.h"
#include "Socket.h"

class EventLoop;

class Channel
{
private:
    int fd_=-1;                             // Channel拥有的fd，Channel和fd是一对一的关系。
    EventLoop *loop_=nullptr;
    bool inepoll_=false;              // Channel是否已添加到epoll树上，如果未添加，调用epoll_ctl()的时候用EPOLL_CTL_ADD，否则用EPOLL_CTL_MOD。
    uint32_t events_=0;              // fd_需要监视的事件。listenfd和clientfd需要监视EPOLLIN，clientfd还可能需要监视EPOLLOUT。
    uint32_t revents_=0;             // fd_已发生的事件。 
    std::function<void()> readcallback_;         // fd_读事件的回调函数，如果是acceptchannel，将回调Acceptor::newconnection()，如果是clientchannel，将回调Connection::onmessage()。
    std::function<void()> closecallback_;
    std::function<void()> errorcallback_;
    std::function<void()> writecallback_;
public:
    Channel(EventLoop* loop,int fd);      // 构造函数。
    ~Channel();                           // 析构函数。 

    int fd();                                            // 返回fd_成员。
    void useet();                                    // 采用边缘触发。
    void enablereading();                     // 让epoll_wait()监视fd_的读事件。
    void disablereading();                    // 取消读事件。
    void enablewriting();                      // 注册写事件。
    void disablewriting();                     // 取消写事件。

    void disableall();                             // 取消全部的事件。
    void remove();                                // 从事件循环中删除Channel。
    void setinepoll(bool inepoll);         // 设置inepoll_成员的值。
    
    void setrevents(uint32_t ev);         // 设置revents_成员的值为参数ev。
    bool inpoll();                                  // 返回inepoll_成员。
    uint32_t events();                           // 返回events_成员。
    uint32_t revents();                          // 返回revents_成员。
    void handleevent();          //事件处理函数，epoll_wait返回的时候执行他

    // void newconnection(Socket* servsock);         //处理新链接的客户端
    
    void setreadcallback(std::function<void()> fn);    // 设置fd_读事件的回调函数。
    void setclosecallback(std::function<void()> fn);
    void seterrorcallback(std::function<void()> fn);
    void setwritecallback(std::function<void()> fn);

};