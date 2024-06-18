#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
using namespace std;
#include "config.h"
int main(int argc, char *argv[])
{
    
    Config *p=Config::Getinstance();
    p->Load("configure");
    string IP=("43.139.2.42");
    int Port=p->GetIntDefault("ListenPort",9999);

    int sockfd;
    struct sockaddr_in servaddr;
    char buf[1024];
 
    if ((sockfd=socket(AF_INET,SOCK_STREAM,0))<0) { printf("socket() failed.\n"); return -1; }
    
    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(Port);
    servaddr.sin_addr.s_addr=inet_addr(IP.c_str());

    if (connect(sockfd, (struct sockaddr *)&servaddr,sizeof(servaddr)) != 0)
    {
        //printf("connect(%s:%s) failed.\n",argv[1],argv[2]); close(sockfd);  return -1;
        cout<<"链接失败"<<endl;
    }

    printf("connect ok.\n");
    // printf("开始时间：%d",time(0));
     

    for (int ii=0;ii<200000;ii++)
    {
        // 从命令行输入内容。
        memset(buf,0,sizeof(buf));
        printf("please input:"); scanf("%s",buf);

        if (send(sockfd,buf,strlen(buf),0) <=0)       // 把命令行输入的内容发送给服务端。
        { 
            printf("write() failed.\n");  close(sockfd);  return -1;
        }
        
        memset(buf,0,sizeof(buf));
        if (recv(sockfd,buf,sizeof(buf),0) <=0)      // 接收服务端的回应。
        { 
            printf("read() failed.\n");  close(sockfd);  return -1;
        }

        printf("recv:%s\n",buf);
    }

    // printf("结束时间：%d",time(0));
} 



