#include "TcpServer.h"

TcpServer::TcpServer(const std::string &ip,const uint16_t port,int threadnum):threadnum_(threadnum)
{
    mainloop_=new EventLoop;
    mainloop_->setepolltimeoutcallback(std::bind(&TcpServer::epolltimeout,this,std::placeholders::_1));   // 设置timeout超时的回调函数。

    acceptor_=new Acceptor(mainloop_,ip,port);
    acceptor_->setnewconnectioncb(std::bind(&TcpServer::newconnection,this,std::placeholders::_1));
    
    threadpool_=new ThreadPool(threadnum_,"IO");
    for (int ii=0;ii<threadnum_;ii++)
    {
        subloops_.push_back(new EventLoop);              // 创建从事件循环，存入subloops_容器中。
        subloops_[ii]->setepolltimeoutcallback(std::bind(&TcpServer::epolltimeout,this,std::placeholders::_1));   // 设置timeout超时的回调函数。
        threadpool_->addtask(std::bind(&EventLoop::run,subloops_[ii]));    // 在线程池中运行从事件循环。
    }
}   

TcpServer::~TcpServer()
{
    delete acceptor_;
    delete mainloop_;
    for(auto &b:conns_)
    {
        delete b.second;
    }
    // 释放从事件循环。
    for (auto &aa:subloops_)
        delete aa;

    delete threadpool_;   // 释放线程池。
}

// 运行事件循环。
void TcpServer::start()          
{
    mainloop_->run();
}
void TcpServer::newconnection(Socket*clientsock)
{
    
    Connection *conn=new Connection(subloops_[clientsock->fd()%threadnum_],clientsock);   
    conn->setclosecallback(std::bind(&TcpServer::closeconnection,this,std::placeholders::_1));
    conn->seterrorcallback(std::bind(&TcpServer::errorconnection,this,std::placeholders::_1));
    conn->setonmessagecallback(std::bind(&TcpServer::onmessage,this,std::placeholders::_1,std::placeholders::_2));
    conn->setsendcompletecallback(std::bind(&TcpServer::sendcomplete,this,std::placeholders::_1));
    printf ("new connection(fd=%d,ip=%s,port=%d) ok.\n",conn->fd(),conn->ip().c_str(),conn->port());
    conns_[conn->fd()]=conn;

    if (newconnectioncb_) newconnectioncb_(conn);             // 回调EchoServer::HandleNewConnection()。
}


 // 关闭客户端的连接，在Connection类中回调此函数。 
 void TcpServer::closeconnection(Connection *conn)
 {
    if (closeconnectioncb_) closeconnectioncb_(conn);       // 回调EchoServer::HandleClose()。

    // printf("client(eventfd=%d) disconnected.\n",conn->fd());
    // close(conn->fd());            // 关闭客户端的fd。
    conns_.erase(conn->fd());   // 从map中删除conn。
    delete conn;
 }

// 客户端的连接错误，在Connection类中回调此函数。
void TcpServer::errorconnection(Connection *conn)
{
    if (errorconnectioncb_) errorconnectioncb_(conn);     // 回调EchoServer::HandleError()。

    //printf("client(eventfd=%d) error.\n",conn->fd());
    conns_.erase(conn->fd());   // 从map中删除conn。
    delete conn;
}

void TcpServer::onmessage(Connection* conn,std::string& message)
{
    /*
    //假设此处经过若干运算
    message="reply:"+message;
    int len=message.size();
    std::string tmpbuf((char*)&len,4);
    tmpbuf.append(message);
    send(conn->fd(),tmpbuf.data(),tmpbuf.size(),0);
    */
   if(onmessagecb_)
   onmessagecb_(conn,message);
}


// 数据发送完成后，在Connection类中回调此函数。
void TcpServer::sendcomplete(Connection *conn)     
{
    printf("send complete.\n");

    // 根据业务的需求，在这里可以增加其它的代码。
    if(sendcompletecb_)
    sendcompletecb_(conn);
}

// epoll_wait()超时，在EventLoop类中回调此函数。
void TcpServer::epolltimeout(EventLoop *loop)         
{
    printf("epoll_wait() timeout.\n");

    // 根据业务的需求，在这里可以增加其它的代码。
    if(timeoutcb_)
    timeoutcb_(loop);
}


void TcpServer::setnewconnectioncb(std::function<void(Connection*)> fn)
{
    newconnectioncb_=fn;
}

void TcpServer::setcloseconnectioncb(std::function<void(Connection*)> fn)
{
    closeconnectioncb_=fn;
}

void TcpServer::seterrorconnectioncb(std::function<void(Connection*)> fn)
{
    errorconnectioncb_=fn;
}

void TcpServer::setonmessagecb(std::function<void(Connection*,std::string &message)> fn)
{
    onmessagecb_=fn;
}

void TcpServer::setsendcompletecb(std::function<void(Connection*)> fn)
{
    sendcompletecb_=fn;
}

void TcpServer::settimeoutcb(std::function<void(EventLoop*)> fn)
{
    timeoutcb_=fn;
}
