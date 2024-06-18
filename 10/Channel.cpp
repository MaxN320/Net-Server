#include "Channel.h"

Channel::Channel(EventLoop *loop, int fd) : loop_(loop), fd_(fd)
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
void Channel::setinepoll()
{
    inepoll_ = true;
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

    if (revents_ & EPOLLRDHUP) // 对方已关闭，有些系统检测不到，可以使用EPOLLIN，recv()返回0。
    {
        printf("client(eventfd=%d) disconnected.\n", fd_);
        close(fd_); // 关闭客户端的fd。
    } //  普通数据  带外数据
    else if (revents_ & (EPOLLIN | EPOLLPRI)) // 接收缓冲区中有数据可以读。
    {
        {
            readcallback_();
        }
        
    }
    else if (revents_ & EPOLLOUT) // 有数据需要写，暂时没有代码，以后再说。
    {
    }
    else // 其它事件，都视为错误。
    {
        printf("client(eventfd=%d) error.\n", fd_);
        close(fd_); // 关闭客户端的fd。
    }
}
void Channel::setreadcallback(std::function<void()> fn)
{
    readcallback_=fn;
}
void Channel::newconnection(Socket* servsock)
{
     InetAddress clientaddr;             // 客户端的地址和协议。
    // 注意，clientsock只能new出来，不能在栈上，否则析构函数会关闭fd。
    // 还有，这里new出来的对象没有释放，这个问题以后再解决。
    Socket *clientsock=new Socket(servsock->accept(clientaddr));

    printf ("accept client(fd=%d,ip=%s,port=%d) ok.\n",clientsock->fd(),clientaddr.ip(),clientaddr.port());   
    Channel *clientchannel=new Channel(loop_,clientsock->fd());   // 这里new出来的对象没有释放，这个问题以后再解决。
    clientchannel->setreadcallback(std::bind(&Channel::onmessage,clientchannel));
    clientchannel->useet();                 // 客户端连上来的fd采用边缘触发。
    clientchannel->enablereading();   // 让epoll_wait()监视clientchannel的读事件

}

void Channel::onmessage()
{
    char buffer[1024];
    while (true)
    {
        bzero(buffer,sizeof(buffer));
        ssize_t nread=read(fd_,buffer,sizeof(buffer));
        if (nread > 0)      // 成功的读取到了数据。
        {
            // 把接收到的报文内容原封不动的发回去。
            printf("recv(eventfd=%d):%s\n",fd_,buffer);
            send(fd_,buffer,strlen(buffer),0);
        } 
        else if (nread == -1 && errno == EINTR) // 读取数据的时候被信号中断，继续读取。
        {  
            continue;
        } 
        else if (nread == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) // 全部的数据已读取完毕。
        {
            break;
        } 
        else if (nread == 0)  // 客户端连接已断开。
        {  
            printf("client(eventfd=%d) disconnected.\n",fd_);
            close(fd_);            // 关闭客户端的fd。
            break;
        }
    }
    
}