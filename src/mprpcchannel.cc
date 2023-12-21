#include "mprpcchannel.h"
#include "rpcheader.pb.h"
#include "mprpcapplication.h"

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string>

void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor *method,
                        google::protobuf::RpcController *controller, const google::protobuf::Message *request,
                        google::protobuf::Message *response, google::protobuf::Closure *done) {
    
    const google::protobuf::ServiceDescriptor *sd = method->service();
    std::string service_name = sd->name();
    std::string method_name = method->name();
    uint32_t args_size = 0;
    std::string args_str;
    if(request->SerializeToString(&args_str)) {
        args_size = args_str.size();
    }
    else {
        controller->SetFailed("serialize request failed!");
        return;
    }


    // 定义rpc的请求header
    mprpc::RpcHeader rpcHeader;
    rpcHeader.set_service_name(service_name);
    rpcHeader.set_method_name(method_name);
    rpcHeader.set_args_size(args_size);

    uint32_t header_size = 0;
    std::string rpc_header_str; 
    if(rpcHeader.SerializeToString(&rpc_header_str)) {
        header_size = rpc_header_str.size();
    } 
    else {
        controller->SetFailed("serialize rpcHeader failed!");
        return;
    }

    std::string send_rpc_str;
    send_rpc_str.insert(0, (char*)&header_size, 4);
    send_rpc_str += rpc_header_str;
    send_rpc_str += args_str;

    // std::cout << "================================================" << std::endl;
    // std::cout << "header_size: " << header_size << std::endl;
    // std::cout << "rpc_header_str: " << rpc_header_str << std::endl;
    // std::cout << "service_name: " << service_name << std::endl;
    // std::cout << "method_name: " << method_name << std::endl;
    // std::cout << "args_size: " << args_size << std::endl;
    // std::cout << "args_str: " << args_str << std::endl;
    // std::cout << "================================================" << std::endl;

    // 使用tcp编程完成rpc远程调用
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);

    if(-1 == clientfd) {
        char errTxt[512];
        sprintf(errTxt, "create clientfd error! errno:%d", errno);
        controller->SetFailed(errTxt);
        return;
    }
    // 读取配置文件rpcserver的信息
    // std::string ip = MprpcApplication::GetInstance()->Getconfig().Load("rpcserverip");
    // uint16_t port = atoi(MprpcApplication::GetInstance()->Getconfig().Load("rpcserverport").c_str());
    ZkClient zkCli;
    zkCli.Start();
    std::string method_path = "/" + service_name + "/" + method_name;
    std::string ip_port = zkCli.GetData(method_path.c_str());
    if(ip_port == "") {
        controller->SetFailed(method_path + "is not exist!");
        return;
    }
    size_t index = ip_port.find(':');
    if(index == std::string::npos) {
        controller->SetFailed(ip_port + "address is invalid!");
        return ;
    }

    std::string ip = ip_port.substr(0, index);
    uint16_t port = atoi(ip_port.substr(index + 1).c_str());

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof server_addr);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    // inet_aton(ip.c_str(), &server_addr.sin_addr);
    inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr);

    if(-1 == connect(clientfd, (struct sockaddr*)&server_addr, sizeof server_addr)) {
        close(clientfd);
        char errTxt[512];
        sprintf(errTxt, "connect error! errno:%d", errno);
        controller->SetFailed(errTxt);
        return;
    }

    if(-1 == send(clientfd, send_rpc_str.c_str(), send_rpc_str.size(), 0)) {
        close(clientfd);
        char errTxt[512];
        sprintf(errTxt, "sned error! errno:%d", errno);
        controller->SetFailed(errTxt);
        return;
    }

    char buf[1024] = {0};
    ssize_t n = 0;
    // 
    if(-1 == (n =recv(clientfd, buf, sizeof buf - 1, 0))) {
        close(clientfd);
        char errTxt[512];
        sprintf(errTxt, "recv error! errno:%d", errno);
        controller->SetFailed(errTxt);
        return;
    }
    
    if(!response->ParseFromArray(buf, n)) {
        close(clientfd);
        char errTxt[512];
        sprintf(errTxt, "parse error! errno:%d", errno);
        controller->SetFailed(errTxt);
        return;
    }
    close(clientfd);
}