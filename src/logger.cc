#include "logger.h"
#include "timestamp.h"
#include <time.h>
#include <iostream>
#include <fstream>

Logger* Logger::m_logger = new Logger();

Logger::Logger() {
    std::thread writeLogTask ([&]() {
        while(true) {

            std::string file_name = Timestamp::now().toStringYear();
            file_name += "-log.txt";

            std::ofstream out(file_name, std::ofstream::out | std::ofstream::app);
            if(!out.is_open()) {
                std::cout << "logger file : " << file_name << " open failed!" << std::endl;
                exit(EXIT_FAILURE);
            }

            std::string msg = m_lckQue.pop();
            out.write(msg.c_str(), msg.size());
            out.close();
        }
    });
    
    writeLogTask.detach();
}


Logger* Logger::GetInstance() {
    return m_logger;
}

void Logger::SetLogLevel(LogLevel level) {
    m_logLevel = level;
}

void Logger::Log(std::string msg) {
    std::string msg_header;
    switch (m_logLevel) {
        case INFO:
            msg_header = "[INFO] ";
            break;
        case ERROR:
            msg_header = "[ERROR] ";
            break;
        default:
            break;
    }
    msg_header += Timestamp::now().toStringDay();
    msg_header += " ===> ";
    msg_header += msg;
    msg_header += '\n';
    m_lckQue.Push(msg_header);
}