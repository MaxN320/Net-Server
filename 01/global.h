#pragma once

typedef struct 
{
    char ItemName[25];
    char ItemContent[200];
}CConfItem,*LPCConfiItem;

//和设置标题有关的全局量
char **g_os_argv;            //原始命令行参数数组,在main中会被赋值
char *gp_envmem = NULL;      //指向自己分配的env环境变量的内存
int  g_environlen = 0;       //环境变量所占内存大小

//字符串相关函数
void Rtrim(char *string);
void Ltrim(char *string);

//设置可执行程序标题相关函数
void ngx_init_setproctitle();
void ngx_setproctitle(const char *title);