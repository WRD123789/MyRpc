#pragma once

#include <unordered_map>
#include <string>

// 用于框架读取配置文件
class MyrpcConfig {
public:
    // 解析配置加载文件
    void loadConfigFile(const char *configFile);
    // 查询配置项信息
    std::string load(const std::string &key);

private:
    // 去掉字符串前后的空格
    void trim(std::string &str);

    std::unordered_map<std::string, std::string> _configMap;
};