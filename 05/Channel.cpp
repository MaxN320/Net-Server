#include "Channel.h"

Channel::Channel(Epoll * ep,int fd):ep_(ep),fd_(fd)
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
    events_|=EPOLLET;
}

void Channel::enablereading()
{
    events_|=EPOLLIN;
    ep_->updateChannel(this);
}
void Channel::setinepoll()
{
    inepoll_=true;
}

bool Channel::inpoll()                                  // 返回inepoll_成员。
{
    return inepoll_;
}

uint32_t Channel::events()                           // 返回events_成员。
{
    return events_;
}


void Channel::setrevents(uint32_t ev)
{
    revents_=ev;
}


uint32_t Channel::revents()                          // 返回revents_成员。
{
    return revents_;
}
