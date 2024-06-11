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