01 -----
EPOll_WAit返回之后
{
    if（是监听套接字）
    {
        创建新的连接
    }
    else
    {
        if （是断开事件） 断开的处理
        else if(是可读事件) 可读的处理
        else if（是可写事件）可写的处理
        else
            按照错误事件处理
    }
    
}

02 -----
优化了Epoll的处理逻辑：
EPOll_WAit返回之后
{
    if （是断开事件） 断开的处理
    else if(是可读事件) 可读的处理
    {
        if（是监听套接字）创建新的连接
        else             可读的处理
    }
    else if（是可写事件）可写的处理
    else  按照错误事件处理
}


新增了一个InetAddress类 封装了IP地址和端口号。
class InetAddress
{
private:
    sockaddr_in addr_;        // 表示地址协议的结构体。
public:
    InetAddress(const std::string &ip,uint16_t port);      // 如果是监听的fd，用这个构造函数。
    InetAddress(const sockaddr_in addr):addr_(addr){}  // 如果是客户端连上来的fd，用这个构造函数。
    ~InetAddress();

    const char *ip() const;                // 返回字符串表示的地址，例如：192.168.150.128
    uint16_t    port() const;              // 返回整数表示的端口，例如：80、8080
    const sockaddr *addr() const;   // 返回addr_成员的地址，转换成了sockaddr。
};
在主程序中使用INetAddress来表示地址，构造函数中设置sockaddr_in addr_



03 -----
新增了一个 Socket类
将设置套接字属性 和服务器的绑定 监听 连接操作封装到这个类里面

在 Tcpepoll.cpp中使用Socket类来替代  
{
    设置套接字属性 
    服务器的绑定 
    监听 
    连接 
}
创建新连接时也用SOcket类代替


//创建一个非阻塞的Socket
int createnonblocking();

class Socket
{
private:
    const int fd_;    //Socket持有的fd，从构造函数传来
public:
    Socket(int fd);    //构造函数
    ~Socket();

    int fd()const ;  //返回fd_成员
    void setreuseaddr(bool on); //设置SO_REUSEADDR选项，ture打开，false关闭
    void setreuseport(bool on); //设置SO_REUSEPORT
    void settcpnodelay(bool on); //设置SO_NODELAY
    void setkeepalive(bool on);  //设置SO_KEEPALIVE

    void bind(const InetAddress& servaddr); //服务端的socket将调用此函数
    void listen(int nn=128);                
    int accept(InetAddress& clientaddr);   
};


04 -----
添加了一个Epoll类 负责将涉及到EPOll的操作 都封装起来了
(添加监听事件 返回所有的监听事件（有就） )

// Epoll类。
class Epoll
{
private:
    static const int MaxEvents=100;                   // epoll_wait()返回事件数组的大小。
    int epollfd_=-1;                                             // epoll句柄，在构造函数中创建。
    epoll_event events_[MaxEvents];                  // 存放poll_wait()返回事件的数组，在构造函数中分配内存。
public:
    Epoll();                                             // 在构造函数中创建了epollfd_。
    ~Epoll();                                          // 在析构函数中关闭epollfd_。

    void addfd(int fd, uint32_t op);                             // 把fd和它需要监视的事件添加到红黑树上。
    std::vector<epoll_event> loop(int timeout=-1);   // 运行epoll_wait()，等待事件的发生，已发生的事件用vector容器返回。
};

主程序死循环{
    evs = loop();
    for(auto b: evs)
        处理逻辑
    这次是在EPoll::loop中调用int epfd=epoll_wait(); 
    没检测到事件或者检测超时在EPoll：：loop中就已经处理好了
}

05-----新增Channel类


class Epoll;

class Channel
{
private:
    int fd_=-1;                             // Channel拥有的fd，Channel和fd是一对一的关系。
    Epoll *ep_=nullptr;                // Channel对应的红黑树，Channel与Epoll是多对一的关系，一个Channel只对应一个Epoll。
    bool inepoll_=false;              // Channel是否已添加到epoll树上，如果未添加，调用epoll_ctl()的时候用EPOLL_CTL_ADD，否则用EPOLL_CTL_MOD。
    uint32_t events_=0;              // fd_需要监视的事件。listenfd和clientfd需要监视EPOLLIN，clientfd还可能需要监视EPOLLOUT。
    uint32_t revents_=0;             // fd_已发生的事件。 

public:
    Channel(Epoll* ep,int fd);      // 构造函数。
    ~Channel();                           // 析构函数。 

    int fd();                                            // 返回fd_成员。
    void useet();                                    // 采用边缘触发。
    void enablereading();                     // 让epoll_wait()监视fd_的读事件。
    void setinepoll();                            // 把inepoll_成员的值设置为true。
    void setrevents(uint32_t ev);         // 设置revents_成员的值为参数ev。
    bool inpoll();                                  // 返回inepoll_成员。
    uint32_t events();                           // 返回events_成员。
    uint32_t revents();                          // 返回revents_成员。
};


    epoll_event ;用来初始化当前fd所需要的事件信息
    {
        struct epoll_event
                {
                uint32_t events;   //要监听的事件
                epoll_data_t data;  // 联合体
                }
    }
    typedef union  epoll_data{
                void *ptr;
                int fd;
                uint32_t u32;
                uint64_t u64;
    }epoll_data_t 

Channel 之前都是用 epoll_data.fd 来直接通信
Channel 之后用 epoll_data.ptr 指向Channel
              再使用Channel类进行携带上一堆
              附加信息，来应对各种事件

使用fd套接字和epoll类 来初始化channel 一个Channel一个fd_  多个Channel一个ep_
fd是一个套接字  EPOLL类 _ep对应多个 fd  fd和ep_ 多对一的关系

{
    {
        有新连接 的话逻辑如下：
        ch.fd=监听  -》 ch.enablereading -> ep_->updateChannel(this) -> 创建event ev.data.ptr=参数
    }
    以前无论是监听套接字的创建 还是 新连接套接字的创建
    我们都直接使用 fd 进行创建    epooll_ctl（）进行控制
    现在我们在中间加入一个 Channel类封装 fd和 此fd对应的操作 
    以后不直接使用fd 而是使用Channel类来创建 
}

    因为此类的出现，epoll_event不在需要，将ep.loop函数返回值 更改为vector<channel>
    
十分的优雅！！！

06 --- 

在channel类中 新增handleevent函数
{
    loop中返回的是 Channel类
    Channel类中有fd 有已经发生的事件  则可以直接对其事件进行处理
    主函数中的处理逻辑 替换成 ch->handleevent函数
}

07 ---
新增一个 std::function<void()> readcallback_; 函数指针
通过 
    void newconnection(Socket* servsock);         //处理新链接的客户端

    void onmessage();                             //处理对端发送过来的数据报
    void setreadcallback(std::function<void()> fn);    // 设置fd_读事件的回调函数。
函数 直接简化了handleevent。
（handlevent不管读事件是 连接事件还是收消息事件 都调用readcallback_） 简化程序逻辑

08 ----
新增EventLoop类
run函数封装了 运行事件的循环

09 ----
对EvnetLoop类做更新
使其完全替代Epoll类 对Epoll类做了又一层次的封装
channel类使用封装了epoll的EventLoop类初始化

EventLoop类中有个成员变量Epoll类变量

定义TcpServer类 将Eventloop类放入其中 ，在TcpServer类初始化时 创建监听事件,设置监听回调
TcpServer.start 运行ep.run 从而实现事件循环

10  ----新增acceptor类
TcpServer类 职责拆分 将职责分给一个新的类Acceptor类
Acceptor类负责 创建监听套接字，注册监听事件以及监听回调。

11 ----新增Connection类
Channel类的职责拆分 将职责分给一个新的Conncetion类
Connection类负责 创建套接字，注册读事件以及处理读事件 在构造函数中注册写事件
Connection
{
    EventLoop *loop_;   //要添加到 此 事件循环
    Socket * clientsock_;  // 文件描述符
    Channel * clientchannel_; // 使用fd套接字和eventloop类进行设置读事件
}

12 ----
职责划分
Acceptor类 应该负责 创建连接套接字并设置新连接套接字的读事件。不应该扔给Channel类
之前是扔给Channel类，优化程序逻辑

13  ----
为Acceptor增加了一个回调函数指针，TcpServer构造时，将已将将TcpServer的NewConnetion函数绑定了上去，使用 占位符的技术
最终 Acceptor也不负责创建Connection

还是职责分化，TcpServeer创建Accptor和Connection 显然更加合理


// 此时流程
// 1.  TcpServer使用命令行参数获取ip以及端口号 
//     {    
            accept_ = new Acceptor(loop_,ip,port);
            Acceptor的初始化中 使用TcpServer传进来的 Eventloop_ ip port
            {
                根据ip和port生成InetAddress成员
                生成一个Socket成员
                创建acceptchannel将传进来的loop_ 和 Socket的fd绑定到一起
                为当前acceptchannel类设置 读的回调函数 
                    acceptchannel_->setreadcallback(std::bind(&Acceptor::newconnection,this));
                    acceptchannel_->enablereading();       // 让epoll_wait()监视servchannel的读事件。 
                    {
                        loop_->updatechannel(this);
                        {
                            ep_->updateChannel(ch);
                            {
                                为当前EPoll类添加了一个读事件epoll_event ev;
                                        ev.data.ptr=ch;将channel和ptr绑定到一块
                            }
                        }
                    }
            }
//     }
// 2.  注册完成之后，运行TcpServer.start函数
//      {
            loop_.run();  //仅此一行
            {
                通过先前设置的ev.data.ptr=ch 来取出channel对象，fd和channel绑定时，注册了读事件的处理函数
                调用channel类的handlevent函数  主要负责处理各种事件 读事件就调用读回调函数

                如果有读事件（连接事件） 通过先前设置好的读回调函数 ->  函数指针 与 回调函数
                -> Channel  std::function<void()> readcallback_;    
                -> Acceptor std::function<void(Socket*)> newconnectioncb_;

                channel::readcallback_  -> Acceptor::newconnection
                Acceptor::newconnectioncb_ -> TcpServer::newconnection(Socket*clientsock)
            }
        }

    3.  如果有连接事件到来，先调用 channel::readcallback_  -> Acceptor::newconnection 
        再调用Acceptor::newconnectioncb_ -> TcpServer::newconnection(Socket*clientsock)
    TcpServer::newconnection(Socket*clientsock)
    {
        Connection *conn=new Connection(&loop_,clientsock);   // 这里new出来的对象没有释放，这个问题以后再解决  仅此一句
        {
            通过Acceptor类中的servsock来 accpet（）；
            clientchannel_->setreadcallback(std::bind(&Channel::onmessage,clientchannel_));设置fd_读事件的回调函数。
            将当前fd添加到红黑树上
        }
    }

    3.  继续执行 loop_.run();  

14 ----
整体逻辑没有变化
为了在TcpServer中定义了map对象 map<文件描述符,Connection*>
在Socket中定义 ip,port  (bind中 和 accept中)
在Connection中定义 ip port函数 （使用socket的ip，port）

15 ----
Channel 类 新增两个函数指针     closecallback errorcallback
新增三个函数
{
    onmessage() //处理对方发来的信息
    setclosecallback(函数指针)
    seterrorcallback(函数指针)
}
将原来的出错 关闭 设置成为回调函数
又是一次封装，有需要只需要在回调函数中设计即可


16  ------
Connection类 新增两个函数指针  
    std::function<void(Connection*)> closecallback_;                   // 关闭fd_的回调函数，将回调TcpServer::closeconnection()。
    std::function<void(Connection*)> errorcallback_;                   // fd_发生了错误的回调函数，将回调TcpServer::errorconnection()。
新增两个设置函数指针的函数

TcpServer类新增两个函数 在newconnection中为新连接设置错误 关闭fd_的回调函数，将回调TcpServer类中的处理函数 如下：
void closeconnection(Connection *conn);  // 关闭客户端的连接，在Connection类中回调此函数。 
void errorconnection(Connection *conn);  // 客户端的连接错误，在Connection类中回调此函数。

让TcpServer类来处理Connection成员变量的错误以及断开。彻底管理COnnection类的生命周期

更加完善了


17-----
新增Buffer类（String类的封装 用来做缓冲区）
每个connection类 新增两个成员变量 输入缓冲区 输出缓冲区
新增一个函数 onmessage（） 将channel类的channel删除

