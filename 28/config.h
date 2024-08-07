#include "global.h"
#include <vector>


class Config
{
public:
    static Config* Getinstance();
    ~Config();
private:
    Config();
public:
    bool Load(const char *pconfName);//装在配置文件
    const char * GetString(const char *p_itemname);
    int GetIntDefault(const char *p_itemname,const int def);
    std::vector<LPCConfiItem> m_ConfigItemList;  // 存储配置信息的表
};
