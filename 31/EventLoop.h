#pragma once
#include <functional>
#include "Epoll.h"
#include "Channel.h"
class Channel;
class Epoll;
#include <memory>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/eventfd.h>
#include <queue>
#include <mutex>

#include <sys/timerfd.h>      // 定时器需要包含这个头文件。

class EventLoop{
private:
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

public:
    EventLoop(bool mainloop);  //构造函数中创建Epoll对象ep_
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
};