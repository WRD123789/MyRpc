#pragma once

#include "myrpcconfig.h"
#include "myrpcchannel.h"
#include "myrpccontroller.h"

// myrpc 框架的基础类
class MyrpcApplication {
public:
    static MyrpcApplication& getInstance();

    void init(int argc, char **argv);
    MyrpcConfig& getConfig();

private:
    MyrpcApplication();
    MyrpcApplication(const MyrpcApplication&) = delete;
    MyrpcApplication(MyrpcApplication&&) = delete;

    MyrpcConfig _config;
};