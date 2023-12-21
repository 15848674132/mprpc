#ifndef MPRPCAPPLICATION_H
#define MPRPCAPPLICATION_H

#include "mprpcconfig.h"
#include "mprpcchannel.h"
#include "mprpccontroller.h"
#include "zookeeperutil.h"

class MprpcApplication {
public:
    static MprpcApplication* GetInstance();
    static void Init(int argc, char* argv[]);
    MprpcConfig& Getconfig() {return m_config; }
private:
    MprpcApplication() = default;
    MprpcApplication(const MprpcApplication&) = delete;
    MprpcApplication& operator=(const MprpcApplication&) = delete;
private:
    static MprpcApplication* m_app;
    MprpcConfig m_config;
};

#endif