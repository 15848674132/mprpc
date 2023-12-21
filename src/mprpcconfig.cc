#include "mprpcconfig.h"
#include "logger.h"
#include <iostream>
#include <fstream>

// 读取配置文件
void MprpcConfig::LoadConigFile(const std::string &configfile) {
    std::ifstream in(configfile);
    if(!in.is_open()) {
        LOG_ERROR("%s:%s:%d ===> %s is not exist!", __FILE__, __FUNCTION__, __LINE__, configfile.c_str());
        exit(EXIT_FAILURE);
    }

    // 按行读取 
    std::string line;
    while(std::getline(in, line)) {
        // 1.去除读取上来的空格
        size_t index = line.find_first_not_of(' ');
        if(index != std::string::npos) 
            line = line.substr(index, line.size() - index);
        
        index = line.find_last_not_of(' ');
        if(index != std::string::npos) 
            line = line.substr(0, index + 1);
        
        // 2.去除注释
        if(line.empty() || line[0] == '#') 
            continue;
        
        // 3.提取配置信息
        index = line.find('=');
        if(index == std::string::npos) 
            continue;
        m_configMap.insert({line.substr(0, index), line.substr(index + 1)}); 
    }

}
// 查询配置项信息
std::string MprpcConfig::Load(const std::string& key) {
    auto iter = m_configMap.find(key);
    if(iter != m_configMap.end()) 
        return iter->second;
    return "";
}