#include "TcpServer.h"

TcpServer::TcpServer(const std::string &ip,const uint16_t port)
{
    acceptor_=new Acceptor(&loop_,ip,port);
}   

TcpServer::~TcpServer()
{
    delete acceptor_;
}

// 运行事件循环。
void TcpServer::start()          
{
    loop_.run();
}