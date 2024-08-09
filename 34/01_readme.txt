
31 -----
主要任务：解决Connection无效的连接（超时检测，太久没发消息，判定为死连接）
EventLoop类新增成员
int timerfd_;                                        // 定时器的fd。
std::unique_ptr<Channel> timerchannel_;              // 定时器的Channel。
bool mainloop_;                                      // true-是主事件循环，false-是从事件循环。


void handletimer();                                                 // 闹钟响时执行的函数。

将EventLoop的timerfd添加到从事件循环 注册读事件 void handletimer();               

Connection类新增成员
Timestamp lastatime_;             // 时间戳，创建Connection对象时为当前时间，每接收到一个报文，把时间戳更新为当前时间。
每次处理报文时（onemessage）就调用语句     lastatime_=Timestamp::now();             // 更新Connection的时间戳。


32-------
TcpServer新增mutex锁
Acceptor会回调 TCPServer::newconnection 往conns_ 添加变量
Connections连接断开 错误 会删除conns_ 里面变量      线程不安全

// 删除conns_中的Connection对象，在EventLoop::handletimer()中将回调此函数。
void TcpServer::removeconn(int fd)                 
{
    printf("TcpServer::removeconn() thread is %d.\n",syscall(SYS_gettid)); 
    {
         std::lock_guard<std::mutex> gd(mmutex_);
        conns_.erase(fd);          // 从map中删除conn。
    }
}

Connection添加timeout函数
拿现在的事件戳对比上一次接受报文的时间戳。判断是否超时连接


EventLoop类添加成员

    int  timetvl_;                                                             // 闹钟时间间隔，单位：秒。。
    int  timeout_;

    std::mutex mmutex_;                                              // 保护conns_的互斥锁。
    std::map<int,spConnection> conns_;                      // 存放运行在该事件循环上全部的Connection对象。
    std::function<void(int)>  timercallback_;                 // 删除TcpServer中超时的Connection对象，将被设置为TcpServer::removeconn()
    // 1、在事件循环中增加map<int,spConnect> conns_容器，存放运行在该事件循环上全部的Connection对象。
    // 2、如果闹钟时间到了，遍历conns_，判断每个Connection对象是否超时。
    // 3、如果超时了，从conns_中删除Connection对象；
    // 4、还需要从TcpServer.conns_中删除Connection对象。
    // 5、TcpServer和EventLoop的map容器需要加锁。
    // 6、闹钟时间间隔和超时时间参数化。


    void newconnection(spConnection conn);            // 把Connection对象保存在conns_中。
    void settimercallback(std::function<void(int)> fn);  // 将被设置为TcpServer::removeconn()  // 从TcpServer的map中删除超时的conn。

改写handtimer，其调用for循环查找当前事件循环的所有的COnnection 判断是否超时




34-----
为buffer类新增了 报文类型 成员
报文为0 则表示没有任何报文头部
报文为1 4字节报文头部
可以继续自定以


// 从buf_中拆分出一个报文，存放在ss中，如果buf_中没有报文，返回false。
bool Buffer::pickmessage(std::string &ss)                           
{
    if (buf_.size()==0) return false;

    if (sep_==0)                  // 没有分隔符。
    {
        ss=buf_;
        buf_.clear();
    }
    else if (sep_==1)          // 四字节的报头。
    {
        int len;
        memcpy(&len,buf_.data(),4);             // 从buf_中获取报文头部。

        if (buf_.size()<len+4) return false;     // 如果buf_中的数据量小于报文头部，说明buf_中的报文内容不完整。

        ss=buf_.substr(4,len);                        // 从buf_中获取一个报文。
        buf_.erase(0,len+4);                          // 从buf_中删除刚才已获取的报文。
    }

    return true;
}