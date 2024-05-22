#include "Epoll.h"

Epoll::Epoll()
{
    if((epollfd_ = epoll_create(1))==-1)
    {
        printf("epoll_cretor() failed(%d).\n",errno);
        exit(1);
    }    

};

Epoll::~Epoll()
{
    close(epollfd_);
}

void Epoll::addfd(int fd,uint32_t op)
{
    epoll_event ev;
    ev.data.fd=fd;
    ev.events=op;     //epoll要监视的fd

    if (epoll_ctl(epollfd_,EPOLL_CTL_ADD,fd,&ev)==-1)     // 把需要监视的fd和它的事件加入epollfd中。
    {
        printf("epoll_ctl() failed(%d).\n",errno); 
        exit(-1);
    }
}

// 运行epoll_wait()，等待事件的发生，已发生的事件用vector容器返回。
std::vector<epoll_event> Epoll::loop(int timeout)   
{
    std::vector<epoll_event> evs;        // 存放epoll_wait()返回的事件。

    bzero(events_,sizeof(events_));
    int infds=epoll_wait(epollfd_,events_,MaxEvents,timeout);       // 等待监视的fd有事件发生。

    // 返回失败。
    if (infds < 0)
    {
        perror("epoll_wait() failed"); exit(-1);
    }

    // 超时。
    if (infds == 0)
    {
        printf("epoll_wait() timeout.\n"); return evs;
    }

    // 如果infds>0，表示有事件发生的fd的数量。
    for (int ii=0;ii<infds;ii++)       // 遍历epoll返回的数组events_。
    {
        evs.push_back(events_[ii]);
    }

    return evs;
}