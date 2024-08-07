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


18-----
为了解决报文的粘包半包问题，改写了onmessage函数

Connection类添加回调函数指针，和回调函数并且将onmessage函数 
剪切到了TcpServer类中

TcpServer类在添加新的Connection变量时，设置Connection的回调发送信息指针()。使用了占位符的技术std::placeholders::_1

现在的处理流程：Connection类
通过 Acceptor类来创建Connection类，创建Connection类时，就将Connection类的回调函数设置好
channel调用为其设置好的回调函数，于是回调到Connection类的Connection::onmessage函数中(读取好了数据，将要发送的数据传参给 TcpServer::onmessage)
Connection::onmessage中又回调了TcpServer::TcpServer::onmessage（这个函数负责处理信息，添加头，并且发送的功能）


19------
channel类中新增 写事件 以及写事件的函数指针初始化为（COnnecton::onmessage）
如果可读就调用 COnnection::onmessage
Connection::onmessage中回调TCPServer::onmessage
然后调用 Connection::send函数{
    保存到发送缓冲区 注册写事件
}
然后TcpServer中的start函数就会运行
{
    抓到写事件，然后调用channel.writecallback
    继续回调
}

20------
没做什么大的改动
在Connection类中新增了一个 发送完成函数指针  sendcompeltecallback_
writecallback中如果发送完成 在设置disablewrtie后 调用sendcompeletecallback_;

    void sendcomplete(Connection *conn);     // 数据发送完成后，在Connection类中回调此函数。{具体实现字写}
    void epolltimeout(EventLoop *loop);         // epoll_wait()超时，在EventLoop类中回调此函数。 {具体实现字写}
Tcpserver中每一个connection都要设置 sendcomplete(Connection *conn); 
为loop设置epolltimeout(EventLoop *loop);      


21------
基本没做改动

添加了一个上层应用层 主要业务是回声服务器。
原本TcpServer的事情 通通扔给EchoServer了。

23------
添加了ThreadPool类{
    正常线程池 没啥内容
}


24-----
创建多个事件循环
一个主事件循环 多个从事件循环
主事件循环负责 监听端口，当来新连接时。
新连接自动放入从事件循环{
    从事件循环绑定ep.run函数
    {
        对Connection连接 发送过来的报文进行 读取 提取 经过特定的计算 发送给Connection
    }
}
25------
在EchoServer中新增加了一个线程池 名为工作线程

24的从事件循环{
    从事件循环绑定ep.run函数
    {
        对Connection连接 发送过来的报文进行 读取 提取 经过特定的计算 发送给Connection
    }
}


25的从事件循环{
    从事件循环绑定ep.run函数
    {
        对Connection连接 发送过来的报文进行 读取 提取 然后向上传递给EchoServer
    }
}

EchoServer的线程池
{
    经过特定的计算 发送给Connection
}
将原来的HandleMessage函数分解成两个函数
{
    HandleMessage负责添加到线程池的任务队列
    onMessage负责 计算 发送
}

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
    conn->send(message.data(),message.size());   // 把数据发送出去。
 }


26----
基本上什么都没做。  展示 之前版本存在的问题：
1。 如果计算流程过长，客户端提前退出。则EchoServer会析构COnnection连接。
2.  使用空连接 造成未知效果


工作线程发送数据前，先打印 sleep（1） ，printf("处理完业务后，将使用connecion对象。\n");
Connection析构前 打印 COnnection即将析构

client发送完数据 不接受。直接退出 sleep（2）  return


27---------
修改上一节的问题

Channel类增加remove函数
{
    disableall();                                // 先取消全部的事件。
    loop_->removechannel(this);    // 从红黑树上删除fd。
    EventLoop类中增加removechannel(Channel *ch)
    {
        ep_->removechannel(ch);
        ep中增加removechannel函数
        {
            if (ch->inpoll())         // 如果channel已经在树上了。
            {
                printf("removechannel()\n");
                if (epoll_ctl(epollfd_,EPOLL_CTL_DEL,ch->fd(),0)==-1)
                {
                    perror("epoll_ctl() failed.\n"); exit(-1);
                }
            }
        }
    }
}

当connection类
// TCP连接关闭（断开）的回调函数，调用remove
// TCP连接错误的回调函数，调用remove。

Conenction类中普通指针都改成智能指针
这样不用手工delete。则不会出现使用野指针的情况
但是还有线程安全的问题，工作线程调用send（）会往output里面写数据
i/o线程将output里面的数据发送出去


28------
没有太大变化
基本上是对指针操作的修改 
{   
    将普通指针换成了智能指针
    有些类的指针换成了局部变量
}

29------
性能优化
{
    eventloop是有限的，通通在TcpServer的构造函数中创建
    每一个创建connection的时候，都将使用这些eventloop。然而这里用的全是智能指针。开销大且没有必要
    因为Tcpserver是智能指针。TcpServer会负责析构。

    所以channel类 Acceptor类 COnnection类中不再使用智能指针，改用裸指针。
    这是性能上的优化。
    
}

30------
EchoServer线程池是工作线程，专门为了工作 为了计算回显业务
TcpServer线程池是I/O线程，专门为了发送数据 初始化时将其设置成EventLoop的run函数 将新连接均匀分配给TcpSrever的线程池
TcpServer线程池负责 接受新连接的发送 与 处理 与 回复数据的任务
{
    clientchanel ——》Connection ——》TcpServer ——》将计算添加到（Echoserver）工作线程 ——》 计算回显业务 ——》COnnection::send函数
    ——》将计算好的数据放入输出缓冲区，注册写事件 ——》写事件触发 ——》实际发送数据 （Connection::）
}

工作线程会调用conn.send函数  放输出缓冲区 注册写事件
I/O线程监听事件循环，有连接的写事件，则会将输出缓冲区的内容发送出去
                    有连接的读事件，则会调用工作线程，工作线程会更改输出缓冲区的内容
则 线程不安全

此讲解决了线程不安全的问题：通过在EventLoop中添加元素
是I/O线程则调用send

    初始化为 
    wakeupfd_(eventfd(0,EFD_NONBLOCK)
    wakechannel_(new Channel(this,wakeupfd_))
{
    wakechannel_->setreadcallback(std::bind(&EventLoop::handlewakeup,this));    //设置读回调函数
    wakechannel_->enablereading();                                              //添加到监听事件上
}
是工作线程调用void EventLoop::queueinloop(std::function<void()> fn) 
    入队后wakeup写入1  write（）； 则可读 调用下面函数

 // 事件循环线程被eventfd唤醒后执行的函数。
 void EventLoop::handlewakeup()
 {
    printf("handlewakeup() thread id is %d.\n",syscall(SYS_gettid));

    uint64_t val;
    read(wakeupfd_,&val,sizeof(val));       // 从eventfd中读取出数据，如果不读取，eventfd的读事件会一直触发。

    std::function<void()> fn;

    std::lock_guard<std::mutex> gd(mutex_);           // 给任务队列加锁。

    // 执行队列中全部的发送任务。
    while (taskqueue_.size()>0)
    {
        fn=std::move(taskqueue_.front());    // 出队一个元素。
        taskqueue_.pop();                              
        fn();                                                    // 执行任务。
    }
 }
    
