#include "rpcprovider.h"
#include "myrpcapplication.h"
#include "rpcheader.pb.h"
#include "logger.h"
#include "zookeeperutil.h"

#include <functional>
#include <google/protobuf/descriptor.h>
#include <iostream>

void RpcProvider::notifyService(google::protobuf::Service *service)
{
    ServiceInfo serviceInfo;
    serviceInfo._service = service;
    // 获取服务对象的描述信息
    auto pServiceDesc = service->GetDescriptor();

    std::string serviceName = pServiceDesc->name();
    int methodCnt = pServiceDesc->method_count();

    LOG_INFO("notify -> serviceName: %s", serviceName.c_str());

    for (int i = 0; i < methodCnt; i += 1) {
        auto pMethodDesc = pServiceDesc->method(i);
        std::string methodName = pMethodDesc->name();
        serviceInfo._methodMap.insert({methodName, pMethodDesc});

        LOG_INFO("notify -> methodName: %s::%s", serviceName.c_str(), 
            methodName.c_str());
    }

    _serverMap.insert({serviceName, serviceInfo});
}


void RpcProvider::run()
{
    MyrpcConfig config = MyrpcApplication::getInstance().getConfig();
    std::string ip = config.load("rpcserverip");
    uint16_t port = atoi(config.load("rpcserverport").c_str());
    muduo::net::InetAddress address(ip, port);

    muduo::net::TcpServer server(&_eventLoop, address, "RpcProvider");

    // 绑定连接回调和消息读写回调方法
    server.setConnectionCallback(std::bind(&RpcProvider::onConnection, 
        this, std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::onMessage, this, 
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    // 设置 muduo 库的线程数量
    server.setThreadNum(4);

    // 将当前 RPC 节点上要发布的服务注册到 zookeeper 上
    ZkClient zkClient;
    zkClient.start();

    // 将 serviceName 设置为永久节点, 将 methodName 设置为临时节点
    for (auto &sp : _serverMap) {
        std::string servicePath = "/" + sp.first;
        zkClient.create(servicePath.c_str(), nullptr, 0);

        for (auto &mp : sp.second._methodMap) {
            std::string methodPath = servicePath + "/" + mp.first;
            char data[128] = {0};
            sprintf(data, "%s:%d", ip.c_str(), port);
            zkClient.create(methodPath.c_str(), data, sizeof(data), ZOO_EPHEMERAL);
        }
    }

    // 启动网络服务
    server.start();
    _eventLoop.loop();
}

void RpcProvider::onConnection(const muduo::net::TcpConnectionPtr &conn)
{
    if (!conn->connected())
        conn->shutdown();
}

void RpcProvider::onMessage(const muduo::net::TcpConnectionPtr &conn,
    muduo::net::Buffer *buffer, muduo::Timestamp time)
{
    // 格式: headerSize (4 字节) + headerStr + argsStr
    std::string recvBuf = buffer->retrieveAllAsString();

    // 从字符串中读取前 4 个字节的数据
    uint32_t headerSize = 0;
    recvBuf.copy((char*)&headerSize, 4, 0);

    std::string rpcHeaderStr = recvBuf.substr(4, headerSize);
    myrpc::RpcHeader rpcHeader;
    std::string serviceName, methodName;
    uint32_t argsSize;
    if (rpcHeader.ParseFromString(rpcHeaderStr)) {
        // 数据头反序列化成功
        serviceName = rpcHeader.servicename();
        methodName = rpcHeader.methodname();
        argsSize = rpcHeader.argssize();
    } else {
        // 数据头反序列化失败
        LOG_ERR("rpcHeaderStr: %s parse error!", rpcHeaderStr.c_str());
        return;
    }

    std::string argsStr = recvBuf.substr(4 + headerSize, argsSize);
    
    auto it = _serverMap.find(serviceName);
    if (it == _serverMap.end()) {
        LOG_ERR("service %s is not exist!", serviceName.c_str());
        return;
    }

    ServiceInfo serviceInfo = it->second;
    auto mit = serviceInfo._methodMap.find(methodName);
    if (mit == serviceInfo._methodMap.end()) {
        LOG_ERR("%s:%s is not exist!", serviceName.c_str(), methodName.c_str());
        return;
    }
    
    // 获取 service 对象和 method 对象
    google::protobuf::Service *service = serviceInfo._service;
    const google::protobuf::MethodDescriptor *method = serviceInfo._methodMap[methodName];

    // 生成 RPC 方法调用的 request 和 response 参数
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    if (!request->ParseFromString(argsStr)) {
        LOG_ERR("request parse error, content: %s", argsStr.c_str());
        return;
    }
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();

    // 给下面的 method 方法绑定一个 Closure 的回调函数
    google::protobuf::Closure *done = google::protobuf::NewCallback<
        RpcProvider, const muduo::net::TcpConnectionPtr&, google::protobuf::Message*>(
            this, &RpcProvider::sendRpcResponse, conn, response);

    // 在框架上, 根据远程 RPC 请求, 调用当前 RPC 节点上发布的方法
    service->CallMethod(method, nullptr, request, response, done);
}

void RpcProvider::sendRpcResponse(const muduo::net::TcpConnectionPtr &conn, 
        google::protobuf::Message *response)
{
    std::string responseStr;
    if (response->SerializeToString(&responseStr))
        conn->send(responseStr);
    else
        LOG_ERR("serialize responseStr error!");
    // 短连接, 主动断开连接
    conn->shutdown();
}