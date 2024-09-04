#include "zookeeperutil.h"
#include "myrpcapplication.h"
#include <semaphore.h>
#include <iostream>

// 在会话状态改变、节点变更时会自动被调用
void globalWatcher(zhandle_t *zh, int type, int state, 
    const char *path, void *watcherCtx)
{
    if (ZOO_SESSION_EVENT == type) {
        if (ZOO_CONNECTED_STATE == state) {
            sem_t *sem = (sem_t*)zoo_get_context(zh);
            sem_post(sem);
        }
    }
}

ZkClient::ZkClient()
    : _zhandle(nullptr)
{}

ZkClient::~ZkClient()
{
    // 关闭句柄, 释放资源
    if (_zhandle != nullptr)
        zookeeper_close(_zhandle);
}

// zookeeper_mt (多线程版本) 的客户端程序会提供三个线程:
// API 调用线程 (主线程)、网络 I/O 线程、watcher 回调线程
void ZkClient::start()
{
    std::string host = MyrpcApplication::getInstance()
        .getConfig().load("zookeeperip");
    std::string port = MyrpcApplication::getInstance()
        .getConfig().load("zookeeperport");
    std::string connStr = host + ":" + port;

    _zhandle = zookeeper_init(connStr.c_str(), globalWatcher, 
        30000, nullptr, nullptr, 0);
    if (nullptr == _zhandle) {
        std::cout << "zookeeper init error!" << std::endl;
        exit(EXIT_FAILURE);
    }

    sem_t sem;
    sem_init(&sem, 0, 0);
    zoo_set_context(_zhandle, &sem);

    sem_wait(&sem);
    std::cout << "zookeeper init success!" << std::endl;
}

void ZkClient::create(const char *path, const char *data, int dataLen, int state)
{
    char pathBuffer[128];
    int bufferLen = sizeof(pathBuffer);
    int flag;

    // 如果指定 znode 已存在, 则无需重复创建
    flag = zoo_exists(_zhandle, path, 0, nullptr);
    if (ZNONODE == flag) {
        // 创建一个临时性 znode 节点
        flag = zoo_create(_zhandle, path, data, dataLen, &ZOO_OPEN_ACL_UNSAFE, 
            state, pathBuffer, bufferLen);
        
        if (ZOK == flag) {
            std::cout << "znode create success, path: " << path << std::endl;
        } else {
            std::cout << "znode create error, path: " << path << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

std::string ZkClient::getData(const char *path)
{
    char buffer[64];
    int bufferLen = sizeof(buffer);

    int flag = zoo_get(_zhandle, path, 0, buffer, &bufferLen, nullptr);
    if (ZOK == flag) {
        return buffer;
    } else {
        std::cout << "get znode error, path: " << path << std::endl;
        return "";
    }
}