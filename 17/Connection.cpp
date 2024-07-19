#include "Connection.h"
Connection::Connection(EventLoop*loop,Socket*clientsock):loop_(loop),clientsock_(clientsock)
{
    clientchannel_=new Channel(loop_,clientsock_->fd());
    clientchannel_->setreadcallback(std::bind(&Connection::onmessage,this));
    clientchannel_->setclosecallback(std::bind(&Connection::closecallback,this));
    clientchannel_->seterrorcallback(std::bind(&Connection::errorcallback,this));
    clientchannel_->useet();                 // 客户端连上来的fd采用边缘触发。
    clientchannel_->enablereading();   // 让epoll_wait()监视clientchannel的读事件
}

void Connection::onmessage()
{
    char buffer[1024];
    while (true)
    {
        bzero(buffer,1024);
        ssize_t nread = read(fd(), buffer, sizeof(buffer));
        if (nread > 0)      // 成功的读取到了数据。
        {
            // 把接收到的报文内容原封不动的发回去。
            // printf("recv(eventfd=%d):%s\n",fd(),buffer);
            // send(fd(),buffer,strlen(buffer),0);
            inputbuffer_.append(buffer,nread);      // 把读取的数据追加到接收缓冲区中。
        } 
        else if (nread == -1 && errno == EINTR) // 读取数据的时候被信号中断，继续读取。
        {  
            continue;
        } 
        else if (nread == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) // 全部的数据已读取完毕。
        {
            printf("recv(eventfd=%d):%s\n",fd(),inputbuffer_.data());
            // 在这里，将经过若干步骤的运算。
            outputbuffer_=inputbuffer_;
            // 运算后的结果已存放在outputbuffer_中。
            inputbuffer_.clear();                                                        // 清空readbuffer_。
            send(fd(),outputbuffer_.data(),outputbuffer_.size(),0);   // 把发送缓冲区中的数据直接send()出去。
            break;
        } 
        else if (nread == 0)  // 客户端连接已断开。
        {  
            // printf("client(eventfd=%d) disconnected.\n",fd());
            // close(fd());            // 关闭客户端的fd。
            closecallback();
            break;
        }
    }
    
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
    closecallback_(this);
}

void Connection::errorcallback()                    // TCP连接错误的回调函数，供Channel回调。
{
    errorcallback_(this);        // 关闭客户端的fd。
}


// 设置关闭fd_的回调函数。
void Connection::setclosecallback(std::function<void(Connection*)> fn)    
{
    closecallback_=fn;     // 回调TcpServer::closeconnection()。
}

// 设置fd_发生了错误的回调函数。
void Connection::seterrorcallback(std::function<void(Connection*)> fn)    
{
    errorcallback_=fn;     // 回调TcpServer::errorconnection()。
}