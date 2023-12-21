#ifndef MPRPCCONFIG_H
#define MPRPCCONFIG_H

#include <unordered_map>
#include <string>

// 框架读取配置文件类
class MprpcConfig {
public:
    // 读取配置文件
    void LoadConigFile(const std::string &configfile);
    // 查询配置项信息
    std::string Load(const std::string& key);
private:
    std::unordered_map<std::string, std::string> m_configMap; 
};

#endif 