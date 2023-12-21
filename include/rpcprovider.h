#ifndef RPCPROVIDER_H
#define RPCPROVIDER_H
#include "google/protobuf/service.h"
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpConnection.h>
#include <string>
#include <unordered_map>

// 框架提供专门服务发布rpc网络服务的网络对象
class RpcProvider {
public:
    RpcProvider() = default;
    // 这里是框架提供给外部使用的，可以发布rpc方法的函数接口
    void NotifyService(::google::protobuf::Service *server);

    // 启动rpc的服务节点，开始提供rpc远程调用服务
    void Run();
private:
    // 服务类型信息
    struct ServiceInfo {
        google::protobuf::Service *m_service;
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor*> m_methodMap;
    };

    void OnConnection(const muduo::net::TcpConnectionPtr&);
    void OnMessage(const muduo::net::TcpConnectionPtr&, muduo::net::Buffer*, muduo::Timestamp);
    // Closure的回调操作，用于序列化rpc的响应和网络发生
    void SendRpcResponse(const muduo::net::TcpConnectionPtr&, google::protobuf::Message*);
private:
    muduo::net::EventLoop m_eventLoop;
    // 存储注册成功的服务对象和其服务方法的所有信息
    std::unordered_map<std::string, ServiceInfo> m_serviceMap;

};

#endif