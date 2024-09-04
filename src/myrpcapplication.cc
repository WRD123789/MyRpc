#include "myrpcapplication.h"

#include <iostream>
#include <string>
#include <unistd.h>

void showArgHelp()
{
    std::cout << "Format: command -i <configfile>" << std::endl;
}

MyrpcApplication::MyrpcApplication()
{

}

MyrpcApplication& MyrpcApplication::getInstance()
{
    static MyrpcApplication app;
    return app;
}

void MyrpcApplication::init(int argc, char **argv)
{
    if (argc < 2) {
        showArgHelp();
        exit(EXIT_FAILURE);
    }

    // 解析命令行参数中的选项
    int c = 0;
    std::string configFile;
    while ((c = getopt(argc, argv, "i:")) != -1) {
        switch (c) {
            case 'i':
                configFile = optarg;
                break;
            case '?':
                showArgHelp();
                exit(EXIT_FAILURE);
                break;
            case ':':
                showArgHelp();
                exit(EXIT_FAILURE);
                break;
            default:
                break;
        }
    }

    // 加载配置文件
    _config.loadConfigFile(configFile.c_str());
}

MyrpcConfig& MyrpcApplication::getConfig()
{
    return _config;
}