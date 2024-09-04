#include "myrpcconfig.h"
#include "logger.h"

#include <iostream>
#include <string>

void MyrpcConfig::loadConfigFile(const char *configFile)
{
    FILE *pfile = fopen(configFile, "r");
    if (nullptr == pfile) {
        std::cout << configFile << " is not exist!" << std::endl;
        exit(EXIT_FAILURE);
    }

    while (!feof(pfile)) {
        char buf[512] = {0};
        fgets(buf, 512, pfile);

        std::string srcBuf(buf);
        trim(srcBuf);
        
        // 过滤 # 注释
        if (srcBuf.empty() || '#' == srcBuf[0])
            continue;
        
        // 解析配置项
        int equalIndex = srcBuf.find('=');
        if (-1 == equalIndex)
            continue;
        
        std::string key = srcBuf.substr(0, equalIndex);
        trim(key);

        std::string value;
        int enterIndex = srcBuf.find('\n', equalIndex);
        if (-1 == enterIndex)
            value = srcBuf.substr(equalIndex + 1);
        else
            value = srcBuf.substr(equalIndex + 1, enterIndex - equalIndex - 1);
        trim(value);

        _configMap.insert({key, value});
    }
}

std::string MyrpcConfig::load(const std::string &key)
{
    auto it = _configMap.find(key);
    if (_configMap.end() == it)
        return "";
    return it->second;
}

void MyrpcConfig::trim(std::string &str)
{
    // 去掉开头的空格
    int notSpaceIndex = str.find_first_not_of(' ');
    if (notSpaceIndex != -1)
        str = str.substr(notSpaceIndex);
        
    // 去掉末尾的空格
    notSpaceIndex = str.find_last_not_of(' ');
    if (notSpaceIndex != -1)
        str = str.substr(0, notSpaceIndex + 1);
}