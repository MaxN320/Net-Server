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

EchoServer::EchoServer(const std::string &ip,const uint16_t port,int subthreadnum,int workthreadnum)
                   :tcpserver_(ip,port,subthreadnum),threadpool_(workthreadnum,"WORKS")

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
    printf("EchoServer::HandleNewConnection() thread is %d.\n",syscall(SYS_gettid));

}

void EchoServer::HandleClose(Connection*conn)
{
    std::cout<<"EchoServer conn closed"<<std::endl;
}

void EchoServer::HandleError(Connection*conn)
{
    std::cout<<"EchoServer conn error"<<std::endl;
}

// 处理客户端的请求报文，在TcpServer类中回调此函数。
void EchoServer::HandleMessage(Connection *conn,std::string& message)     
{
    // printf("EchoServer::HandleMessage() thread is %d.\n",syscall(SYS_gettid));
    // 把业务添加到线程池的任务队列中。
    threadpool_.addtask(std::bind(&EchoServer::OnMessage,this,conn,message));
}

 // 处理客户端的请求报文，用于添加给线程池。
 void EchoServer::OnMessage(Connection *conn,std::string& message)     
 {
    // 在这里，将经过若干步骤的运算。
    message="reply:"+message;          // 回显业务。
    sleep(2);
    printf("处理完业务后，将使用connecion对象。\n");
    conn->send(message.data(),message.size());   // 把数据发送出去。 使用野指针的后果是不可预知的。
    //conn->send(message.data(),message.size());   // 把数据发送出去。
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

