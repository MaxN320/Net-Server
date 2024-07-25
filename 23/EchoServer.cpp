#include "EchoServer.h"
/*
class EchoServer
{
private:
    TcpServer tcpserver_;

public:
    EchoServer(const std::string &ip,const uint16_t port);
    ~EchoServer();

    void Start();                // 启动服务。

    void HandleNewConnection(Socket *clientsock);    // 处理新客户端连接请求，在TcpServer类中回调此函数。
    void HandleClose(Connection *conn);  // 关闭客户端的连接，在TcpServer类中回调此函数。 
    void HandleError(Connection *conn);  // 客户端的连接错误，在TcpServer类中回调此函数。
    void HandleMessage(Connection *conn,std::string message);     // 处理客户端的请求报文，在TcpServer类中回调此函数。
    void HandleSendComplete(Connection *conn);     // 数据发送完成后，在TcpServer类中回调此函数。
    void HandleTimeOut(EventLoop *loop);         // epoll_wait()超时，在TcpServer类中回调此函数。
};
*/

EchoServer::EchoServer(const std::string &ip,const uint16_t port):tcpserver_(ip,port)
{
    tcpserver_.setnewconnectioncb(std::bind(&EchoServer::HandleNewConnection,this,std::placeholders::_1));
    tcpserver_.setcloseconnectioncb(std::bind(&EchoServer::HandleClose, this, std::placeholders::_1));
    tcpserver_.seterrorconnectioncb(std::bind(&EchoServer::HandleError, this, std::placeholders::_1));
    tcpserver_.setonmessagecb(std::bind(&EchoServer::HandleMessage, this, std::placeholders::_1, std::placeholders::_2));
    tcpserver_.setsendcompletecb(std::bind(&EchoServer::HandleSendComplete, this, std::placeholders::_1));
    // tcpserver_.settimeoutcb(std::bind(&EchoServer::HandleTimeOut, this, std::placeholders::_1));
}

EchoServer::~EchoServer()
{

}

void EchoServer::Start()
{
    tcpserver_.start();
}

void EchoServer::HandleNewConnection(Connection*conn)
{
    std::cout<<"New Connection Come in"<<endl;
}

void EchoServer::HandleClose(Connection*conn)
{
    std::cout<<"EchoServer conn closed"<<std::endl;
}

void EchoServer::HandleError(Connection*conn)
{
    std::cout<<"EchoServer conn error"<<std::endl;
}

void EchoServer::HandleMessage(Connection* conn,std::string& message)
{
    message="reply:"+message;          // 回显业务。
    /*
    int len=message.size();                   // 计算回应报文的大小。
    std::string tmpbuf((char*)&len,4);  // 把报文头部填充到回应报文中。
    tmpbuf.append(message);             // 把报文内容填充到回应报文中。
    */          
    conn->send(message.data(),message.size());   // 把临时缓冲区中的数据发送出去。
}


// 数据发送完成后，在TcpServer类中回调此函数。
void EchoServer::HandleSendComplete(Connection *conn)     
{
    std::cout << "Message send complete." << std::endl;

    // 根据业务的需求，在这里可以增加其它的代码。
}



/*
// epoll_wait()超时，在TcpServer类中回调此函数。
void EchoServer::HandleTimeOut(EventLoop *loop)         
{
    std::cout << "EchoServer timeout." << std::endl;

    // 根据业务的需求，在这里可以增加其它的代码。
}
*/