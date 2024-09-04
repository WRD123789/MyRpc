#pragma once

#include "logqueue.h"

#include <string>

// LOG_INFO("xxx %d %s", 20, "xxx")
#define LOG_INFO(logmsgformat, ...) \
    do { \
        Logger &logger = Logger::getInstance(); \
        logger.setLogLevel(INFO); \
        char c[1024] = {0}; \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
        logger.log(c); \
    } while(0);

#define LOG_ERR(logmsgformat, ...) \
    do { \
        Logger &logger = Logger::getInstance(); \
        logger.setLogLevel(ERROR); \
        char c[1024] = {0}; \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
        logger.log(c); \
    } while(0);

enum LogLevel {
    INFO, // 普通信息
    ERROR // 错误信息
};

// 日志系统
class Logger {
public:
    static Logger& getInstance();

    void setLogLevel(LogLevel level); // 设置日志的级别
    void log(std::string msg);        // 写日志

private:
    Logger();
    Logger(const Logger&) = delete;
    Logger(Logger&&) = delete;

    int _logLevel;
    LogQueue<std::string> _logQueue; // 日志缓冲队列
};