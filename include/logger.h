#ifndef LOGGER_H
#define LOGGER_H

#include "lockqueue.h"


enum LogLevel {
    INFO,
    ERROR
};

#define LOG_INFO(LogmsgFormat, ...)                       \
    do                                                    \
    {                                                     \
        Logger *logger = Logger::GetInstance();     \
        logger->SetLogLevel(INFO);                        \
        char buf[1024] = {0};                             \
        snprintf(buf, 1024, LogmsgFormat, ##__VA_ARGS__); \
        logger->Log(buf);                                 \
    } while (0)

#define LOG_ERROR(LogmsgFormat, ...)                      \
    do                                                    \
    {                                                     \
        Logger *logger = Logger::GetInstance();     \
        logger->SetLogLevel(ERROR);                       \
        char buf[1024] = {0};                             \
        snprintf(buf, 1024, LogmsgFormat, ##__VA_ARGS__); \
        logger->Log(buf);                                 \
    } while (0)


class Logger {
public:
    static Logger* GetInstance();
    ~Logger() = default;
    void SetLogLevel(LogLevel level);
    void Log(std::string msg);
private:
    Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger &&) = delete;
    Logger& operator=(Logger &&) = delete;
private:
    int m_logLevel;
    LockQueue<std::string> m_lckQue;
    static Logger *m_logger;
};


#endif