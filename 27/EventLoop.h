#pragma once
#include <functional>
#include "Epoll.h"
#include "Channel.h"
class Channel;
class Epoll;

#include <unistd.h>
#include <sys/syscall.h>

class EventLoop{
private:
    Epoll *ep_;  //每个事件循环只有一个Epoll
    std::function<void (EventLoop*)> epolltimeoutcallback_;
public:
    EventLoop();  //构造函数中创建Epoll对象ep_
    ~EventLoop();  //析构函数中销毁ep_

    void run();   //运行事件循环
    void updatechannel(Channel*ch);
    void setepolltimeoutcallback(std::function<void (EventLoop*)>fn);

    void removechannel(Channel *ch);                       // 从黑树上删除channel。
};