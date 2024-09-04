#include "user.pb.h"
#include "myrpcapplication.h"
#include "rpcprovider.h"
#include "logger.h"

#include <iostream>
#include <string>

class UserService : public fixbug::UserServiceRpc {
public:
    bool login(std::string name, std::string pwd)
    {
        std::cout << "Doing local service: login" << std::endl;
        std::cout << "name: " << name << "pwd: " << pwd << std::endl;

        return true;
    }

    bool regis(uint32_t id, std::string name, std::string pwd)
    {
        std::cout << "Doing local service: regis" << std::endl;
        std::cout << "id: " << id << " name: " << name << " pwd: " << pwd << std::endl;

        return true;
    }

    void login(::google::protobuf::RpcController *controller, 
        const ::fixbug::LoginRequest *request, 
        ::fixbug::LoginResponse *response,
        ::google::protobuf::Closure *done)
    {
        std::string name = request->name();
        std::string pwd = request->pwd();

        // 在本地执行业务
        bool login_result = login(name, pwd);

        // 写入响应
        fixbug::ResultCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_success(login_result);

        // 执行回调
        done->Run();
    }

    void regis(::google::protobuf::RpcController *controller, 
        const ::fixbug::RegisterRequest *request, 
        ::fixbug::RegisterResponse *response,
        ::google::protobuf::Closure *done)
    {
        uint32_t id = request->id();
        std::string name = request->name();
        std::string pwd = request->pwd();

        // 在本地执行业务
        bool regis_result = regis(id, name, pwd);

        // 写入响应
        fixbug::ResultCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_success(regis_result);

        // 执行回调
        done->Run();
    }
};

int main(int argc, char **argv)
{
    // 完成自定义 RPC 框架的初始化
    MyrpcApplication::getInstance().init(argc, argv);

    // RPC 网络服务对象, 将 UserService 对象发布到 RPC 节点上
    RpcProvider provider;
    provider.notifyService(new UserService());

    // 启动 RPC 服务, 此后进程进入阻塞状态, 等待 RPC 调用请求
    provider.run();

    return 0;
}