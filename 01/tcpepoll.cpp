#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>          
#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <sys/epoll.h>
#include <netinet/tcp.h>
#include <iostream>
#include "global.h"
#include "config.h"
using namespace std; 


void setnonblocking(int fd)
{
    fcntl(fd,F_SETFL,fcntl(fd,F_GETFL)|O_NONBLOCK);
    //设置为非阻塞的文件描述符
}
int main()
{
    // 创建服务端用于监听的listenfd。
    int listenfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if (listenfd < 0)
    {
        perror("socket() failed"); return -1;
    }

    int opt = 1; 
    setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&opt,static_cast<socklen_t>(sizeof opt));    // 必须的。
    setsockopt(listenfd,SOL_SOCKET,TCP_NODELAY   ,&opt,static_cast<socklen_t>(sizeof opt));    // 必须的。
    setsockopt(listenfd,SOL_SOCKET,SO_REUSEPORT ,&opt,static_cast<socklen_t>(sizeof opt));    // 有用，但是，在Reactor中意义不大。
    setsockopt(listenfd,SOL_SOCKET,SO_KEEPALIVE   ,&opt,static_cast<socklen_t>(sizeof opt));    // 可能有用，但是，建议自己做心跳。

    setnonblocking(listenfd);    // 把服务端的listenfd设置为非阻塞的。

    Config *p=Config::Getinstance();
    p->Load("configure");
    string IP=p->GetString("ListenIp");
    int Port=p->GetIntDefault("ListenPort",9999);


    struct sockaddr_in servaddr;                                  // 服务端地址的结构体。
    servaddr.sin_family = AF_INET;                              // IPv4网络协议的套接字类型。
    servaddr.sin_addr.s_addr = inet_addr(IP.c_str());      // 服务端用于监听的ip地址。
    servaddr.sin_port = htons(Port);
    
    if (bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0 )
    {
        perror("bind() failed"); close(listenfd); return -1;
    }

    if (listen(listenfd,128) != 0 )        // 在高并发的网络服务器中，第二个参数要大一些。
    {
        perror("listen() failed"); close(listenfd); return -1;
    }

    int epollfd=epoll_create(1);        // 创建epoll句柄（红黑树）。

    struct epoll_event ev;
    ev.data.fd=listenfd;
    ev.events=EPOLLIN;
    epoll_ctl(epollfd,EPOLL_CTL_ADD,listenfd,&ev);
    struct epoll_event evs[10];

    while (true)
    {
        int infds=epoll_wait(epollfd,evs,10,-1);
        if(infds<0){
            cout<<"epoll_wait failed";
            break;
        }
        if(infds==0){
            cout<<"epoll_wait超时";
            continue;
        }
        for(int i=0; i<infds;i++){
            if(evs[i].data.fd=listenfd)
            {
                struct sockaddr_in clientaddr;
                socklen_t len = sizeof(clientaddr);
                int clientfd = accept(listenfd,(struct sockaddr*)&clientaddr,&len);
                setnonblocking(clientfd);         // 客户端连接的fd必须设置为非阻塞的。

                printf ("accept client(fd=%d,ip=%s,port=%d) ok.\n",clientfd,inet_ntoa(clientaddr.sin_addr),ntohs(clientaddr.sin_port));
 // 为新客户端连接准备读事件，并添加到epoll中。
                ev.data.fd=clientfd;
                ev.events=EPOLLIN|EPOLLET;           // 边缘触发。
                epoll_ctl(epollfd,EPOLL_CTL_ADD,clientfd,&ev);
                /////////////////////////////////
            }
            else
            {
                if(evs[i].events & EPOLLRDHUP)
                {
                    cout<<"client (eventfd="<<evs[i].data.fd<<") disconnected .\n";
                    close(evs[i].data.fd);
                }
                else if (evs[i].events & (EPOLLIN |EPOLLPRI))
                {
                    char buffer[1024];
                    while (true)
                    {
                        bzero(&buffer,sizeof(buffer));
                        ssize_t nread =read(evs[i].data.fd,buffer,1024);
                        if (nread > 0)      // 成功的读取到了数据。
                        {
                            // 把接收到的报文内容原封不动的发回去。
                            printf("recv(eventfd=%d):%s\n",evs[i].data.fd,buffer);
                            send(evs[i].data.fd,buffer,strlen(buffer),0);
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
                            printf("2client(eventfd=%d) disconnected.\n",evs[i].data.fd);
                            close(evs[i].data.fd);            // 关闭客户端的fd。
                            break;
                        }
                    }
                    
                }
                else if (evs[i].events & EPOLLOUT)                  // 有数据需要写，暂时没有代码，以后再说。
                {
                }
                else                                                                    // 其它事件，都视为错误。
                {
                    printf("3client(eventfd=%d) error.\n",evs[i].data.fd);
                    close(evs[i].data.fd);            // 关闭客户端的fd。
                }
            }
        }
    }
    

    //init_setproctitle();   //给环境变量搬家 
    return 0;
}
