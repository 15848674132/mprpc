#include "mprpcapplication.h"
#include <iostream>
#include <unistd.h>

void ShowArgsHelp() {
    std::cout << "format: command -i <configfile>" << std::endl;
}

MprpcApplication* MprpcApplication::m_app = new MprpcApplication();

MprpcApplication* MprpcApplication::GetInstance() {
    return m_app;
}

void MprpcApplication::Init(int argc, char *argv[]) {
    if(argc < 2) {
        ShowArgsHelp();
        exit(EXIT_FAILURE);
    }  

    int c = 0;
    std::string config_file;
    while((c = getopt(argc, argv, "i:")) != -1) {
        switch (c) {
            case 'i':
                config_file = optarg;
                break;
            case '?':
                ShowArgsHelp();
                break;
            case ':':
                ShowArgsHelp();
                break;
            default:
                ShowArgsHelp();
                break;
        }
    }  

    // 开始加载配置文件 rpcserver_ip rpcserver_port zookeeper_ip zookeeper_port
    MprpcApplication::GetInstance()->m_config.LoadConigFile(config_file);
}