#include "EventLoop.h"

EventLoop::EventLoop() : ep_(new Epoll)
{
    ;
}

EventLoop::~EventLoop()
{
    
}

void EventLoop::run()
{
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