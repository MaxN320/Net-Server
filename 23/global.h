#pragma once

typedef struct 
{
    char ItemName[25];
    char ItemContent[200];
}CConfItem,*LPCConfiItem;

//和设置标题有关的全局量
//字符串相关函数
void Rtrim(char *string);
void Ltrim(char *string);

//设置可执行程序标题相关函数
//void init_setproctitle();
//void ngx_setproctitle(const char *title);
