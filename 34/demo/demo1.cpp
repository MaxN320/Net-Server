/*
 * 程序名：demo01.cpp，此程序用于演示eventfd。
 * 作者：吴从周
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>          
#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <sys/epoll.h> 
#include <sys/eventfd.h>     // eventfd需要包含这个头文件。

int main(int argc,char *argv[])
{
    int efd=eventfd(0,EFD_SEMAPHORE);        // 创建eventfd。  EFD_CLOEXEC|EFD_NONBLOCK|EFD_SEMAPHORE 
    // 第一个参数作为eventfd的计数器使用
    // write(eventfd,&buf,sizeof(uint64_t))  // 将buf里面的内容加到eventfd的内部计数器中
    // read(eventfd,&buf,sizeof(uint64_t))   // 将eventfd内部的计数器-1 , buf值不会变 和 sizeof(uint64_t) 似乎并没有什么作用
    uint64_t buf=2;
    ssize_t ret;
 
    // 写eventfd，buf必须是8字节。
    ret = write(efd, &buf, sizeof(uint64_t));
    ret = write(efd, &buf, sizeof(uint64_t));
 
    // 读eventfd。
    ret = read(efd, &buf, sizeof(uint64_t));
    printf("ret=%d,buf=%d\n",ret,buf);

    ret = read(efd, &buf, sizeof(uint64_t));
    printf("ret=%d,buf=%d\n",ret,buf);
    
    ret = read(efd, &buf, sizeof(uint64_t));
    printf("ret=%d,buf=%d\n",ret,buf);
    
    ret = read(efd, &buf, sizeof(uint64_t));
    printf("ret=%d,buf=%d\n",ret,buf);

    printf("打印一下 buf =%d\n",buf);
    ret = read(efd, &buf, sizeof(uint64_t));
    printf("ret=%d,buf=%d\n",ret,buf);
    ret = read(efd, &buf, sizeof(uint64_t));
    printf("ret=%d,buf=%d\n",ret,buf);
    ret = read(efd, &buf, sizeof(uint64_t));
    printf("ret=%d,buf=%d\n",ret,buf);
    ret = read(efd, &buf, sizeof(uint64_t));
    printf("ret=%d,buf=%d\n",ret,buf);
    ret = read(efd, &buf, sizeof(uint64_t));
    printf("ret=%d,buf=%d\n",ret,buf);

    close(efd);  // 关闭eventfd。

    return 0;
}