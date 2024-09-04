#pragma once

#include "google/protobuf/service.h"

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpConnection.h>
#include <string>
#include <unordered_map>

struct ServiceInfo {
    google::protobuf::Service *_service;
    std::unordered_map<std::string, 
        const google::protobuf::MethodDescriptor*> _methodMap;
};

// 用于发布 RPC 服务
class RpcProvider {
public:
    // 用于发布 RPC 方法
    void notifyService(google::protobuf::Service *service);

    // 启动 RPC 服务节点, 开始提供 RPC 服务
    void run();

private:
    // 连接事件回调
    void onConnection(const muduo::net::TcpConnectionPtr &conn);
    // 读写事件回调
    void onMessage(const muduo::net::TcpConnectionPtr &conn,
        muduo::net::Buffer *buffer, muduo::Timestamp time);
    
    // Closure 的回调操作, 用于序列化 RPC 的响应和网络发送
    void sendRpcResponse(const muduo::net::TcpConnectionPtr &conn, 
        google::protobuf::Message *message);

    muduo::net::EventLoop _eventLoop;
    // 存储注册成功的服务对象及其方法的所有信息
    std::unordered_map<std::string, ServiceInfo> _serverMap;
};