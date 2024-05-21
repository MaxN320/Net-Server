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
#include "InetAddress.h"
#include "Socket.h"
using namespace std;

// char **g_os_argv;            //原始命令行参数数组,在main中会被赋值
// char *gp_envmem = NULL;      //指向自己分配的env环境变量的内存
// int  g_environlen = 0;       //环境变量所占内存大小

int main()
{
    Config *p = Config::Getinstance();
    p->Load("configure");
    string IP = p->GetString("ListenIp");
    int Port = p->GetIntDefault("ListenPort", 9999);
    Socket servsock(createnonblocking());
    InetAddress servaddr(IP.c_str(), Port);
    servsock.setreuseaddr(true);
    servsock.settcpnodelay(true);
    servsock.setreuseport(true);
    servsock.setkeepalive(true);
    servsock.bind(servaddr);
    servsock.listen();

    int epollfd = epoll_create(1); // 创建epoll句柄（红黑树）。

    struct epoll_event ev;
    ev.data.fd = servsock.fd();
    ev.events = EPOLLIN;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, servsock.fd(), &ev);
    struct epoll_event evs[10];

    while (true)
    {
        int infds = epoll_wait(epollfd, evs, 10, -1);
        if (infds < 0)
        {
            cout << "epoll_wait failed";
            break;
        }
        if (infds == 0)
        {
            cout << "epoll_wait超时";
            continue;
        }
        for (int i = 0; i < infds; i++)
        {
            if (evs[i].events & EPOLLRDHUP)
            {
                cout << "client (eventfd=" << evs[i].data.fd << ") disconnected .\n";
                close(evs[i].data.fd);
            }
            if (evs[i].events & (EPOLLIN | EPOLLPRI))
            {
                if (evs[i].data.fd == servsock.fd())
                {
                    
                    InetAddress clientaddr;   //将客户端的地址 信息放入Inetaddress
                    //这里使用SOcket类来获取新连接的fd
                    //Socket类只能new，否则会自动析构
                    Socket * clientsock=new Socket(servsock.accept(clientaddr));
                    printf ("accept client(fd=%d,ip=%s,port=%d) ok.\n",clientsock->fd(),clientaddr.ip(),clientaddr.port());

                    // 为新客户端连接准备读事件，并添加到epoll中。
                    ev.data.fd = clientsock->fd();
                    ev.events = EPOLLIN | EPOLLET; // 边缘触发。
                    epoll_ctl(epollfd, EPOLL_CTL_ADD, clientsock->fd(), &ev);
                    /////////////////////////////////
                }
                else
                {
                    char buffer[1024];
                    while (true)
                    {
                        bzero(&buffer, sizeof(buffer));
                        ssize_t nread = read(evs[i].data.fd, buffer, 1024);
                        if (nread > 0) // 成功的读取到了数据。
                        {
                            // 把接收到的报文内容原封不动的发回去。
                            printf("recv(eventfd=%d):%s\n", evs[i].data.fd, buffer);
                            send(evs[i].data.fd, buffer, strlen(buffer), 0);
                        }
                        else if (nread == -1 && errno == EINTR) // 读取数据的时候被信号中断，继续读取。
                        {
                            continue;
                        }
                        else if (nread == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) // 全部的数据已读取完毕。
                        {
                            break;
                        }
                        else if (nread == 0) // 客户端连接已断开。
                        {
                            printf("client(eventfd=%d) disconnected.\n", evs[i].data.fd);
                            close(evs[i].data.fd); // 关闭客户端的fd。
                            break;
                        }
                    }
                }
            }
            else if (evs[i].events & EPOLLOUT) // 有数据需要写，暂时没有代码，以后再说。
            {
            }
            else // 其它事件，都视为错误。
            {
                printf("3client(eventfd=%d) error.\n", evs[i].data.fd);
                close(evs[i].data.fd); // 关闭客户端的fd。
            }
        }
    }
    return 0;
}

// init_setproctitle();   //给环境变量搬家
