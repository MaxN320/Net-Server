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
#include "Epoll.h"
#include "EventLoop.h"
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

    EventLoop loop;
    
    Channel * servchannel=new Channel(&loop,servsock.fd());
    servchannel->setreadcallback(std::bind(&Channel::newconnection,servchannel,&servsock));
    servchannel->enablereading();
    loop.run();
    return 0;
}