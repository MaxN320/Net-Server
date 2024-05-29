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

这次是在EPoll::loop中调用int epfd=epoll_wait();
没检测到事件或者检测超时在EPoll：：loop中就已经处理好了
