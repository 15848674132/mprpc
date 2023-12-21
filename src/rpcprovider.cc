#include "rpcprovider.h"
#include "mprpcapplication.h"
#include "zookeeperutil.h"
#include "rpcheader.pb.h"
#include "logger.h"
#include <google/protobuf/descriptor.h>
// #include <google/protobuf/callback.h>

void RpcProvider::NotifyService(google::protobuf::Service *service) {
    ServiceInfo service_info;
    // 获取服务对象的描述信息
    const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();
    // 获取服务名称
    std::string service_name = pserviceDesc->name();
    std::cout << "service_name:" << service_name << std::endl;
    // 获取服务对象service的方法的数量
    int methodCnt = pserviceDesc->method_count();

    for(int i = 0; i < methodCnt; ++i) {
        // 获取服务对象下表的服务方法描述（抽象描述）
        const google::protobuf::MethodDescriptor* pmethodDesc = pserviceDesc->method(i);
        std::string method_name = pmethodDesc->name();
        std::cout << "method_name:" << method_name << std::endl;
        service_info.m_methodMap.insert({method_name, pmethodDesc});
    }

    service_info.m_service = service;
    m_serviceMap.insert({service_name, service_info});
}

// 启动rpc的服务节点，开始提供rpc远程调用服务
void RpcProvider::Run() {
    std::string ip = MprpcApplication::GetInstance()->Getconfig().Load("rpcserverip");
    uint16_t port = atoi(MprpcApplication::GetInstance()->Getconfig().Load("rpcserverport").c_str());
    muduo::net::InetAddress address(ip, port);

    // 创建TcpServer对象
    muduo::net::TcpServer server(&m_eventLoop, address, "RpcProvider");
    // 设置有连接处理时的回调
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection, this, std::placeholders::_1));
    // 设置读写事件的回调
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage, this, 
                              std::placeholders::_1, 
                              std::placeholders::_2,
                              std::placeholders::_3));
    // 设置线程池个数 IO线程
    server.setThreadNum(4);
    // 把当前进程rpc节点上要发布的服务全部注册到zkserver上面， 让rpc client可以从zkserver上面发现服务
    // session timeout 30s zkclient 网络I/O线程 1/3 * timeout 时间发送ping消息
    ZkClient zkCli;
    zkCli.Start();
    // service_name为永久节点 method_name为临时节点
    for(auto &sp : m_serviceMap) {
        std::string service_path = "/" + sp.first;
        zkCli.Create(service_path.c_str(), nullptr, 0);
        for(auto &mp : sp.second.m_methodMap) {
            // /service_name/method_name
            std::string method_path = service_path + "/" + mp.first;
            char method_path_data[128] = {0};
            sprintf(method_path_data, "%s:%d", ip.c_str(), port);
            zkCli.Create(method_path.c_str(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL); 
        }
    }
    
    LOG_INFO("PrcProvider start service at ip:%s port:%d", ip.c_str(), port);
    // 启动线程池
    server.start();
    // 开启listen loop
    m_eventLoop.loop();
}

void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr& conn) {
    if(!conn->connected()) 
        conn->shutdown();
}

/**
 * 在框架内部，RpcProvider和RpcConsumer协商好之间通信用的protobuf数据类型
 * service_name method_name args 定义proto的message类型，进行数据头的序列化和反序列
 * 解决Tcp粘包问题
 * header_size(4 byte) + header_str + args
 * head_str : serviec_name + method_name + args_size
*/

void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr& conn,
                            muduo::net::Buffer* buffer, 
                            muduo::Timestamp) {
    std::string recv_buf = buffer->retrieveAllAsString();
    uint32_t header_size = 0;
    recv_buf.copy((char*)&header_size, 4, 0);
    std::string rpc_header_str = recv_buf.substr(4, header_size);
    mprpc::RpcHeader rpcHeader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size;
    if(rpcHeader.ParseFromString(rpc_header_str)) {
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    }
    else {
        LOG_ERROR("%s:%s:%d ===> Parse rpc_header_str failed!", __FILE__, __FUNCTION__, __LINE__);
        return;
    }

    std::string args_str = recv_buf.substr(4 + header_size, args_size);

    // 获取service对象和method方法
    auto siter = m_serviceMap.find(service_name);
    if(siter == m_serviceMap.end()) {
        LOG_ERROR("%s:%s:%d ===> %s is not exist!", __FILE__, __FUNCTION__, __LINE__, service_name.c_str());
        return;
    }

    auto miter = siter->second.m_methodMap.find(method_name);
    if(miter == siter->second.m_methodMap.end()) {
        LOG_ERROR("%s:%s:%d ===> %s:%s is not exist!", __FILE__, __FUNCTION__, __LINE__, service_name.c_str(), method_name.c_str());
        return;
    }

    google::protobuf::Service *service = siter->second.m_service;
    const google::protobuf::MethodDescriptor *method = miter->second;

    // 生成rpc方法调用的请求request和响应response参数
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    if(!request->ParseFromString(args_str)) {
        LOG_ERROR("%s:%s:%d ===> args_str parse failed!", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();

    google::protobuf::Closure *done = google::protobuf::NewCallback<RpcProvider,
                                                                    const muduo::net::TcpConnectionPtr&,
                                                                    google::protobuf::Message*>
                                      (this, &RpcProvider::SendRpcResponse, conn, response);

    // 在框架上调用远端rpc请求，调用当前rpc节点rpc节点上发布的方法
    service->CallMethod(method, nullptr, request, response, done);
}


void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr& conn, google::protobuf::Message* response) {
    std::string response_str;
    if(response->SerializeToString(&response_str)) 
        conn->send(response_str);
    else 
        LOG_ERROR("%s:%s:%d ===> serialize response failed!", __FILE__, __FUNCTION__, __LINE__);
    conn->shutdown();
}
