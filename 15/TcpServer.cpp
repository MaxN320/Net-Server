#include "TcpServer.h"

TcpServer::TcpServer(const std::string &ip,const uint16_t port)
{
    acceptor_=new Acceptor(&loop_,ip,port);
    acceptor_->setnewconnectioncb(std::bind(&TcpServer::newconnection,this,std::placeholders::_1));
}   

TcpServer::~TcpServer()
{
    delete acceptor_;
    for(auto &b:conns_)
    {
        delete b.second;
    }
}

// 运行事件循环。
void TcpServer::start()          
{
    loop_.run();
}
void TcpServer::newconnection(Socket*clientsock)
{
    Connection *conn=new Connection(&loop_,clientsock);   // 这里new出来的对象没有释放，这个问题以后再解决。
    printf ("new connection(fd=%d,ip=%s,port=%d) ok.\n",conn->fd(),conn->ip().c_str(),conn->port());
    conns_[conn->fd()]=conn;
}