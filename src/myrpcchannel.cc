#include "myrpcchannel.h"
#include "rpcheader.pb.h"
#include "myrpcapplication.h"
#include "myrpccontroller.h"
#include "zookeeperutil.h"

#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>

void MyrpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                google::protobuf::RpcController* controller,
                const google::protobuf::Message* request,
                google::protobuf::Message* response,
                google::protobuf::Closure* done)
{
    const google::protobuf::ServiceDescriptor *serviceDesc = method->service();
    std::string serviceName = serviceDesc->name();
    std::string methodName = method->name();

    // 获取参数序列化后的字符串长度
    int argsSize;
    std::string argsStr;
    if (request->SerializeToString(&argsStr)) {
        argsSize = argsStr.size();
    } else {
        controller->SetFailed("serialize request error!");
        return;
    }

    myrpc::RpcHeader rpcHeader;
    rpcHeader.set_servicename(serviceName);
    rpcHeader.set_methodname(methodName);
    rpcHeader.set_argssize(argsSize);

    uint32_t headerSize;
    std::string rpcHeaderStr;
    if (rpcHeader.SerializeToString(&rpcHeaderStr)) {
        headerSize = rpcHeaderStr.size();
    } else {
        controller->SetFailed("serialize rpc header error!");
        return;
    }

    // 生成 RPC 请求字符串
    std::string sendRpcStr;
    sendRpcStr.insert(0, std::string((char*)&headerSize, 4));
    sendRpcStr += rpcHeaderStr + argsStr;

    // 从 zookeeper 中获取提供指定 RPC 方法的服务器的地址
    ZkClient zkClient;
    zkClient.start();

    std::string methodPath = "/" + serviceName + "/" + methodName;
    std::string host = zkClient.getData(methodPath.c_str());
    if ("" == host) {
        controller->SetFailed(methodPath + " is not exist!");
        return;
    }
    int index = host.find(":");
    if (-1 == index) {
        controller->SetFailed(methodPath + " address is invalid!");
        return;
    }
    std::string ip = host.substr(0, index);
    uint16_t port = atoi(host.substr(index + 1).c_str());

    // 通过 TCP Socket 传输数据
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == clientfd) {
        controller->SetFailed("create socket error! errno: " + errno);
        return;
    }

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = inet_addr(ip.c_str());

    if (-1 == connect(clientfd, (struct sockaddr*)&serverAddress, 
        sizeof(serverAddress))) {
        controller->SetFailed("connect error! errno: " + errno);
        close(clientfd);
        return;
    }

    // 发送 RPC 请求
    if (-1 == send(clientfd, sendRpcStr.c_str(), sendRpcStr.size(), 0)) {
        controller->SetFailed("send error! errno: " + errno);
        close(clientfd);
        return;
    }

    // 接收 RPC 请求的响应值
    char recvBuf[1024] = {0};
    int recvSize = 0;
    if (-1 == (recvSize = recv(clientfd, recvBuf, sizeof(recvBuf), 0))) {
        controller->SetFailed("send error! errno: " + errno);
        close(clientfd);
        return;
    }

    close(clientfd);

    if (!response->ParseFromArray(recvBuf, recvSize)) {
        controller->SetFailed("parse error! responseStr: " + 
            std::string(recvBuf, recvSize));
        return;
    }

    return;
}