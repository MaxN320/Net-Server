#include "Socket.h"

int createnonblocking()
{
    // 创建服务端用于监听的listenfd。
    int listenfd = socket(AF_INET,SOCK_STREAM|SOCK_NONBLOCK,IPPROTO_TCP);
    if(listenfd <0)
    {
        // perror("socket() failed"); exit(-1);
        printf("%s:%s:%d listen socket create error:%d\n", __FILE__, __FUNCTION__, __LINE__, errno); 
        exit(-1);
    }
    return listenfd;
}

Socket::Socket(int fd):fd_(fd)
{

}

Socket::~Socket()
{
    ::close(fd_);
}

int Socket::fd() const 
{
    return fd_;
}
void Socket::settcpnodelay(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval)); // TCP_NODELAY包含头文件 <netinet/tcp.h>
}

void Socket::setreuseaddr(bool on)
{
    int optval=on?1:0;
    ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval); // 必须的。
}


void Socket::setreuseport(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)); 
}

void Socket::setkeepalive(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)); 
}

void Socket::bind(const InetAddress& servaddr)
{
    if(::bind(fd_,servaddr.addr(),sizeof servaddr)<0)
    {
        perror("bind() failed");
        close(fd_);
        exit(-1);
    }
}

void Socket::listen(int nn)
{
    if(::listen(fd_,nn)!=0)
    {
        perror("listen() failed");
        close(fd_);
        exit(-1);
    }
}



int Socket::accept(InetAddress& clientaddr)
{
    sockaddr_in peeraddr;
    socklen_t len = sizeof(peeraddr);
    int clientfd = accept4(fd_,(sockaddr*)&peeraddr,&len,SOCK_NONBLOCK);

    clientaddr.setaddr(peeraddr);             // 客户端的地址和协议。

    return clientfd;    
}