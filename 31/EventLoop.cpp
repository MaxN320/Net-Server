#include "EventLoop.h"

int createtimerfd(int sec=30)
{
    int tfd=timerfd_create(CLOCK_MONOTONIC,TFD_CLOEXEC|TFD_NONBLOCK);   // 创建timerfd。
    struct itimerspec timeout;                                // 定时时间的数据结构。
    memset(&timeout,0,sizeof(struct itimerspec));
    timeout.it_value.tv_sec = 5;                             // 定时时间，固定为5，方便测试。
    timeout.it_value.tv_nsec = 0;
    timerfd_settime(tfd,0,&timeout,0);
    return tfd;
}
// 在构造函数中创建Epoll对象ep_。
EventLoop::EventLoop(bool mainloop)
                  :mainloop_(mainloop),ep_(new Epoll),
                  wakeupfd_(eventfd(0,EFD_NONBLOCK)),
                  wakechannel_(new Channel(this,wakeupfd_)),
                  timerfd_(createtimerfd()),timerchannel_(new Channel(this,timerfd_))
{
    wakechannel_->setreadcallback(std::bind(&EventLoop::handlewakeup,this));
    wakechannel_->enablereading();

    timerchannel_->setreadcallback(std::bind(&EventLoop::handletimer,this));
    timerchannel_->enablereading();
}
EventLoop::~EventLoop()
{
    
}

void EventLoop::run()
{
    threadid_=syscall(SYS_gettid);    // 获取事件循环所在线程的id。
    while (true)
    {
        std::vector<Channel *> channels = ep_->loop();
        if (channels.size() == 0)
        {
            epolltimeoutcallback_(this);
        }
        else
        {

            for (auto &ch : channels)
            {
                ch->handleevent();
            }
        }
    }
}
void EventLoop::updatechannel(Channel *ch)
{
    ep_->updateChannel(ch);
}

void EventLoop::setepolltimeoutcallback(std::function<void(EventLoop *)> fn)
{
    epolltimeoutcallback_ = fn;
}


 // 从黑树上删除channel。
 void EventLoop::removechannel(Channel *ch)                       
 {
    ep_->removechannel(ch);
 }


// 判断当前线程是否为事件循环线程。
bool EventLoop::isinloopthread()   
{
    return threadid_==syscall(SYS_gettid); 
}

 // 把任务添加到队列中。
 void EventLoop::queueinloop(std::function<void()> fn)
 {
    {
        std::lock_guard<std::mutex> gd(mutex_);           // 给任务队列加锁。
        taskqueue_.push(fn);                                            // 任务入队。
    }

    wakeup();        // 唤醒事件循环。
 }

// 用eventfd唤醒事件循环线程。
 void EventLoop::wakeup()
 {
    uint64_t val=1;
    write(wakeupfd_,&val,sizeof(val));
 }

 // 事件循环线程被eventfd唤醒后执行的函数。
 void EventLoop::handlewakeup()
 {
    printf("handlewakeup() thread id is %d.\n",syscall(SYS_gettid));

    uint64_t val;
    read(wakeupfd_,&val,sizeof(val));       // 从eventfd中读取出数据，如果不读取，eventfd的读事件会一直触发。

    std::function<void()> fn;

    std::lock_guard<std::mutex> gd(mutex_);           // 给任务队列加锁。

    // 执行队列中全部的发送任务。
    while (taskqueue_.size()>0)
    {
        fn=std::move(taskqueue_.front());    // 出队一个元素。
        taskqueue_.pop();                              
        fn();                                                    // 执行任务。
    }
 }


// 闹钟响时执行的函数。
void EventLoop::handletimer()                                                 
{
    // 重新计时。
    struct itimerspec timeout;                                // 定时时间的数据结构。
    memset(&timeout,0,sizeof(struct itimerspec));
    timeout.it_value.tv_sec = 5;                             // 定时时间，固定为5，方便测试。
    timeout.it_value.tv_nsec = 0;
    timerfd_settime(timerfd_,0,&timeout,0);

    if (mainloop_)
        printf("主事件循环的闹钟时间到了。\n");
    else
        printf("从事件循环的闹钟时间到了。\n");
}