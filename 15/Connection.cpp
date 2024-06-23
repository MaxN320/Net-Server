#include "Connection.h"
Connection::Connection(EventLoop*loop,Socket*clientsock):loop_(loop),clientsock_(clientsock)
{
    clientchannel_=new Channel(loop_,clientsock_->fd());
    clientchannel_->setreadcallback(std::bind(&Channel::onmessage,clientchannel_));
    clientchannel_->setclosecallback(std::bind(&Connection::closecallback,this));
    clientchannel_->seterrorcallback(std::bind(&Connection::errorcallback,this));
    clientchannel_->useet();                 // 客户端连上来的fd采用边缘触发。
    clientchannel_->enablereading();   // 让epoll_wait()监视clientchannel的读事件
}


Connection::~Connection()
{
    delete clientsock_;
    delete clientchannel_;
}

int Connection::fd() const
{
    return clientsock_->fd();
}
std::string Connection::ip() const{
    return clientsock_->ip();
}
uint16_t Connection::port() const{
    return clientsock_->port();
}

void Connection::closecallback()
{
    printf("client(eventfd=%d) disconnected.\n",fd());
    close(fd());            // 关闭客户端的fd
}

void Connection::errorcallback()                    // TCP连接错误的回调函数，供Channel回调。
{
    printf("client(eventfd=%d) error.\n",fd());
    close(fd());            // 关闭客户端的fd。
}