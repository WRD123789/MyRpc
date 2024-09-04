#include "logger.h"

#include <time.h>
#include <iostream>

Logger::Logger()
{
    std::thread writeLogThread([&]() {
        for (;;) {
            // 获取当天时间
            time_t now = time(nullptr);
            tm *nowtm = localtime(&now);

            char fileName[128];
            sprintf(fileName, "%d-%d-%d-log.txt", nowtm->tm_year + 1900, 
                nowtm->tm_mon + 1, nowtm->tm_mday);
            
            FILE *pFile = fopen(fileName, "a+");
            if (pFile == nullptr) {
                std::cout << "logger file: " << fileName << " open error!" << std::endl;
                exit(EXIT_FAILURE);
            }

            std::string msg = _logQueue.pop();
            // 获取当前的时分秒
            char timeStr[20];
            sprintf(timeStr, "%d:%d:%d => [%s] ", nowtm->tm_hour, 
                nowtm->tm_min, nowtm->tm_sec, (_logLevel == INFO ? "info" : "error"));
            msg.insert(0, timeStr);
            msg.append("\n");
            fputs(msg.c_str(), pFile);
            fclose(pFile);
        }
    });
    writeLogThread.detach();
}

Logger& Logger::getInstance()
{
    static Logger logger;
    return logger;
}

void Logger::setLogLevel(LogLevel level)
{
    _logLevel = level;
}

void Logger::log(std::string msg)
{
    _logQueue.push(msg);
}