#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "config.h"

Config *Config::Getinstance()
{
    static Config m_instance;

    return &m_instance;
}
Config::Config()
{
}
Config::~Config()
{

    /*
    for (auto obj : m_ConfigItemList)
    {
        delete obj;
    }
    m_ConfigItemList.clear();
    */
}

bool Config::Load(const char *pconfName)
{
    FILE *fp;
    fp = fopen(pconfName, "r");
    if (fp == NULL)
        return false;
    char linebuf[226];
    while (!feof(fp))
    {
       
            /// 从文件中读数据，每次读一行，一行最多不要超过225个字符
            if (fgets(linebuf, 225, fp) == NULL)
                continue;
            if (linebuf[0] == 0 || linebuf[0] == '[')
                continue;
            if (*linebuf == ';' || *linebuf == ' ' || *linebuf == '#' || *linebuf == '\n' || *linebuf == '\t')
                continue;

        deleuseless:
            if (strlen(linebuf) > 0)
            {
                if (linebuf[strlen(linebuf) - 1] == 10 || linebuf[strlen(linebuf) - 1] == 13 || linebuf[strlen(linebuf) - 1] == 32)
                {
                    linebuf[strlen(linebuf) - 1] = 0;
                    goto deleuseless;
                }
            }

            char *ptmp = strchr(linebuf, '=');
            if (ptmp != NULL)
            {
                LPCConfiItem P_configitem = new CConfItem();
                memset(P_configitem, 0, sizeof(CConfItem));
                strncpy(P_configitem->ItemName, linebuf, (int)(ptmp - linebuf)); // 等号左侧的拷贝到p_confitem->ItemName
                strcpy(P_configitem->ItemContent, ptmp + 1);
                Ltrim(P_configitem->ItemName);
                Rtrim(P_configitem->ItemContent);
                Ltrim(P_configitem->ItemContent);
                Rtrim(P_configitem->ItemName);
                m_ConfigItemList.push_back(P_configitem);
            }
        
        
    }
    fclose(fp);
    return true;
}

// 根据ItemName获取配置信息字符串，不修改不用互斥
const char *Config::GetString(const char *p_itemname)
{
    std::vector<LPCConfiItem>::iterator pos;
    for (pos = m_ConfigItemList.begin(); pos != m_ConfigItemList.end(); ++pos)
    {
        if (strcasecmp((*pos)->ItemName, p_itemname) == 0)
            return (*pos)->ItemContent;
    } // end for
    return NULL;
}
// 根据ItemName获取数字类型配置信息，不修改不用互斥
int Config::GetIntDefault(const char *p_itemname, const int def)
{
    std::vector<LPCConfiItem>::iterator pos;
    for (pos = m_ConfigItemList.begin(); pos != m_ConfigItemList.end(); ++pos)
    {
        if (strcasecmp((*pos)->ItemName, p_itemname) == 0)
            return atoi((*pos)->ItemContent);
    } // end for
    return def;
}
