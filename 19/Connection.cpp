#include "Connection.h"
Connection::Connection(EventLoop*loop,Socket*clientsock):loop_(loop),clientsock_(clientsock)
{
    clientchannel_=new Channel(loop_,clientsock_->fd());
    clientchannel_->setreadcallback(std::bind(&Connection::onmessage,this));
    clientchannel_->setclosecallback(std::bind(&Connection::closecallback,this));
    clientchannel_->seterrorcallback(std::bind(&Connection::errorcallback,this));
    clientchannel_->setwritecallback(std::bind(&Connection::writecallback,this));

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
            while (true)
            {
                int len;
                memcpy(&len,inputbuffer_.data(),4);
                if(inputbuffer_.size()<len+4)
                break;
                std::string message(inputbuffer_.data()+4,len);
                inputbuffer_.erase(0,len+4);

                printf("message (eventfd=%d):%s\n",fd(),message.c_str());
                onmessagecallback_(this,message);
            }
            
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


// 设置处理报文的回调函数。
void Connection::setonmessagecallback(std::function<void(Connection*,std::string)> fn)    
{
    onmessagecallback_=fn;       // 回调TcpServer::onmessage()。
}

void Connection::send(const char* data,size_t size)
{
    outputbuffer_.append(data,size);
    clientchannel_->enablewriting();
}

void Connection::writecallback()
{
     int writen=::send(fd(),outputbuffer_.data(),outputbuffer_.size(),0);    // 尝试把outputbuffer_中的数据全部发送出去。
    if (writen>0) outputbuffer_.erase(0,writen);                                        // 从outputbuffer_中删除已成功发送的字节数。

    // 如果发送缓冲区中没有数据了，表示数据已发送完成，不再关注写事件。
    if (outputbuffer_.size()==0) clientchannel_->disablewriting();        
}