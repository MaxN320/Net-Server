#include "Channel.h"
#include "Connection.h"

Channel::Channel(const std::unique_ptr<EventLoop>& loop,int fd):loop_(loop),fd_(fd)      // 构造函数。
{

}


Channel::~Channel()
{
    // 在析构函数中，不要销毁ep_，也不能关闭fd_，因为这两个东西不属于Channel类，Channel类只是需要它们，使用它们而已。
}

int Channel::fd()
{
    return fd_;
}

void Channel::useet()
{
    events_ |= EPOLLET;
}

void Channel::enablereading()
{
    events_ |= EPOLLIN;
    loop_->updatechannel(this);
}
void Channel::disablereading()
{
    events_&=~EPOLLIN;
    loop_->updatechannel(this);
}
void Channel::enablewriting()
{
    events_!=EPOLLOUT;
    loop_->updatechannel(this);
}
void Channel::disablewriting()
{
    events_&= ~EPOLLOUT;
    loop_->updatechannel(this);
}

void Channel::disableall()                             // 取消全部的事件。
{
    events_=0;
    loop_->updatechannel(this);
}

void Channel::remove()                                // 从事件循环中删除Channel。
{
    disableall();                                // 先取消全部的事件。
    loop_->removechannel(this);    // 从红黑树上删除fd。
}

void Channel::setinepoll(bool inepoll)                           // 设置inepoll_成员的值。
{
    inepoll_=inepoll;
}

bool Channel::inpoll() // 返回inepoll_成员。
{
    return inepoll_;
}

uint32_t Channel::events() // 返回events_成员。
{
    return events_;
}

void Channel::setrevents(uint32_t ev)
{
    revents_ = ev;
}

uint32_t Channel::revents() // 返回revents_成员。
{
    return revents_;
}
void Channel::handleevent()
{
   if (revents_ & EPOLLRDHUP)                     // 对方已关闭，有些系统检测不到，可以使用EPOLLIN，recv()返回0。
    {
        closecallback_();      // 回调Connection::closecallback()。
    }                               
    else if (revents_ & (EPOLLIN|EPOLLPRI))   // 接收缓冲区中有数据可以读。
    {
        readcallback_();   // 如果是acceptchannel，将回调Acceptor::newconnection()，如果是clientchannel，将回调Connection::onmessage()。
    }  
    else if (revents_ & EPOLLOUT)                  // 有数据需要写。
    {
        writecallback_();      // 回调Connection::writecallback()。     
    }
    else                                                           // 其它事件，都视为错误。
    {
        errorcallback_();       // 回调Connection::errorcallback()。
    }
}
void Channel::setreadcallback(std::function<void()> fn)
{
    readcallback_=fn;
}

void Channel::setclosecallback(std::function<void()>fn)
{
    closecallback_=fn;
}

void Channel::seterrorcallback(std::function<void ()>fn)
{
    errorcallback_=fn;
}
void Channel::setwritecallback(std::function<void()>fn)
{
    writecallback_=fn;
}