#include "myrpcapplication.h"
#include "user.pb.h"

#include <iostream>

int main(int argc, char **argv)
{
    MyrpcApplication::getInstance().init(argc, argv);

    fixbug::UserServiceRpc_Stub stub(new MyrpcChannel());

    // 设置函数调用参数和返回值
    fixbug::LoginRequest request;
    request.set_name("Test..");
    request.set_pwd("123456");
    fixbug::LoginResponse response;

    MyrpcController controller;

    // 调用发布的 RPC 方法 login
    stub.login(&controller, &request, &response, nullptr);

    if (controller.Failed()) {
        std::cout << controller.ErrorText() << std::endl;
    } else {
        if (0 == response.result().errcode())
            std::cout << "rpc login response success: " << response.success() << std::endl;
        else
            std::cout << "rpc login response error: " << response.result().errmsg() 
                << std::endl;
    }
    
    // 设置函数调用参数和返回值
    fixbug::RegisterRequest request0;
    request0.set_id(1);
    request0.set_name("Test..");
    request0.set_pwd("123456");
    fixbug::RegisterResponse response0;

    controller.Reset();

    // 调用发布的 RPC 方法 regis
    stub.regis(&controller, &request0, &response0, nullptr);

    if (controller.Failed()) {
        std::cout << controller.ErrorText() << std::endl;
    } else {
        if (0 == response0.result().errcode())
            std::cout << "rpc regis response success: " << response.success() << std::endl;
        else
            std::cout << "rpc regis response error: " << response.result().errmsg() 
                << std::endl;
    }

    return 0;
}