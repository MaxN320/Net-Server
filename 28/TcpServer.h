#pragma once
#include <iostream>
#include "EventLoop.h"
#include "Socket.h"
#include "Channel.h"
#include "Acceptor.h"
#include <map>
#include "ThreadPool.h"
#include "Connection.h"
#include <memory>
using namespace std;

class TcpServer
{
private:
    
    std::unique_ptr<EventLoop> mainloop_;                                 // 主事件循环。 祼指针 普通指针 原始指针 std::unique_ptr
    std::vector<std::unique_ptr<EventLoop>> subloops_;            // 存放从事件循环的容器。
    Acceptor acceptor_;                                         // 一个TcpServer只有一个Acceptor对象。
    int threadnum_;                                               // 线程池的大小，即从事件循环的个数。
    ThreadPool threadpool_;                                 // 线程池。
    std::map<int,spConnection> conns_;
    // 一个TcpServer有多个Connection对象，存放在map容器中。

    function<void(spConnection)> newconnectioncb_;
    function<void(spConnection)> closeconnectioncb_;
    function<void(spConnection)> errorconnectioncb_;
    function<void(spConnection,string&message)> onmessagecb_;
    function<void(spConnection)>sendcompletecb_; 
    function<void(EventLoop*)> timeoutcb_;


public:
    TcpServer(const std::string &ip,const uint16_t port,int threadnum=3);
    ~TcpServer();

    void start();

   
    void newconnection(std::unique_ptr<Socket> clientsock);    // 处理新客户端连接请求，在Acceptor类中回调此函数。
    void closeconnection(spConnection conn);  // 关闭客户端的连接，在Connection类中回调此函数。 
    void errorconnection(spConnection conn);  // 客户端的连接错误，在Connection类中回调此函数。
    void onmessage(spConnection conn,std::string &message);

    void sendcomplete(spConnection conn); //发送完成后，Connection中回调此函数
    void epolltimeout(EventLoop * loop); //epollwait超时，在Eventloop中回调此函数


    void setnewconnectioncb(std::function<void(spConnection)> fn);
    void setcloseconnectioncb(std::function<void(spConnection)> fn);
    void seterrorconnectioncb(std::function<void(spConnection)> fn);
    void setonmessagecb(std::function<void(spConnection,std::string &message)> fn);
    void setsendcompletecb(std::function<void(spConnection)> fn);
    void settimeoutcb(std::function<void(EventLoop*)> fn);
};