#pragma once
#include <functional>
#include "Epoll.h"
#include "Channel.h"
#include <memory>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/eventfd.h>
#include <queue>
#include <mutex>

#include <sys/timerfd.h>      // 定时器需要包含这个头文件。
#include <map>
#include "Connection.h"
class Channel;
class Epoll;

class Connection;
using spConnection=std::shared_ptr<Connection>;

class EventLoop{
private:
    int  timetvl_;                                                             // 闹钟时间间隔，单位：秒。。
    int  timeout_;                                                           // Connection对象超时的时间，单位：秒


    std::unique_ptr<Epoll> ep_;                       // 每个事件循环只有一个Epoll。
    std::function<void (EventLoop*)> epolltimeoutcallback_;

    pid_t threadid_;
    std::queue<std::function<void()>> taskqueue_;
    std::mutex mutex_;
    int wakeupfd_;
    std::unique_ptr<Channel> wakechannel_;

    int timerfd_;                                        // 定时器的fd。
    std::unique_ptr<Channel> timerchannel_;              // 定时器的Channel。
    bool mainloop_;                                      // true-是主事件循环，false-是从事件循环。

    std::mutex mmutex_;                                              // 保护conns_的互斥锁。
    std::map<int,spConnection> conns_;                      // 存放运行在该事件循环上全部的Connection对象。
    std::function<void(int)>  timercallback_;                 // 删除TcpServer中超时的Connection对象，将被设置为TcpServer::removeconn()
    // 1、在事件循环中增加map<int,spConnect> conns_容器，存放运行在该事件循环上全部的Connection对象。
    // 2、如果闹钟时间到了，遍历conns_，判断每个Connection对象是否超时。
    // 3、如果超时了，从conns_中删除Connection对象；
    // 4、还需要从TcpServer.conns_中删除Connection对象。
    // 5、TcpServer和EventLoop的map容器需要加锁。
    // 6、闹钟时间间隔和超时时间参数化。

public:
    EventLoop(bool mainloop,int timetvl=30,int timeout=80);  
    ~EventLoop();  //析构函数中销毁ep_

    void run();   //运行事件循环
    void updatechannel(Channel*ch);
    void setepolltimeoutcallback(std::function<void (EventLoop*)>fn);
    void removechannel(Channel *ch);                       // 从黑树上删除channel。


    bool isinloopthread();   // 判断当前线程是否为事件循环线程。

    void queueinloop(std::function<void()> fn);          // 把任务添加到队列中。
    void wakeup();                                                        // 用eventfd唤醒事件循环线程。
    void handlewakeup();                                             // 事件循环线程被eventfd唤醒后执行的函数。

    void handletimer();                                                 // 闹钟响时执行的函数。


    void newconnection(spConnection conn);            // 把Connection对象保存在conns_中。
    void settimercallback(std::function<void(int)> fn);  // 将被设置为TcpServer::removeconn()
};