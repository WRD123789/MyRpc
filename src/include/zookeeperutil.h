#pragma once

#include <semaphore.h>
#include <zookeeper/zookeeper.h>
#include <string>

// 封装的 zookeeper 客户端
class ZkClient {
public:
    ZkClient();
    ~ZkClient();

    // 连接 zkserver
    void start();
    // 在 zkserver 上根据指定的 path 创建 znode 节点
    void create(const char *path, const char *data, int dataLen, int state = 0);
    // 根据参数指定的 znode 节点路径, 获取其中的值
    std::string getData(const char *path);

private:
    // zk 的客户端句柄
    zhandle_t *_zhandle;
};