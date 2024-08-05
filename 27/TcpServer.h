#pragma once
#include <iostream>
#include "EventLoop.h"
#include "Socket.h"
#include "Channel.h"
#include "Acceptor.h"
#include <map>
#include "ThreadPool.h"
#include "Connection.h"
using namespace std;
class TcpServer
{
private:
    EventLoop *mainloop_; //一主事件循环
    vector<EventLoop*> subloops_;
    ThreadPool *threadpool_;
    int threadnum_;
    Acceptor *acceptor_; //一个TcpServer只有一个Accept类
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

    void newconnection(Socket *clientsock);    // 处理新客户端连接请求。
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