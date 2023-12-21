#include <iostream>
#include "mprpcapplication.h"
#include "user.pb.h"


int main(int argc, char *argv[]) {
    // 整个程序启动以后，想使用mprpc框架来享受rpc服务调用，一定要先调用框架的初始化函数（只初始化一次）
    MprpcApplication::Init(argc, argv);
    // 演示调用远程发布的rpc方法Login
    fixbug::UserServiceRpc_Stub stub(new MprpcChannel());
    // rpc方法的请求参数
    fixbug::LoginRequest request;
    request.set_name("zhang san");
    request.set_password("123456");
    // rpc方法的响应
    fixbug::LoginResponse response;
    MprpcController controller;
    // 同步的rpc方法调用 ===> rpcChannel->callMethod()
    stub.Login(&controller, &request, &response, nullptr);

    if(controller.Failed()) {
        std::cout << controller.ErrorText() << std::endl;
    }
    else {
        // rpc 调用完成
        if(response.result().errcode() == 0) {
            std::cout << "rpc login response success: " << response.success() << std::endl;
        }
        else {
            std::cout << "rpc login response error : " << response.result().errmsg() << std::endl;
        }
    }

    fixbug::RegisterRequest request1;
    request1.set_name("li si");
    request1.set_password("666666");
    fixbug::RegisterResponse response1;
    MprpcController controller1;

    stub.Register(&controller1, &request1, &response1, nullptr);
    if(controller1.Failed()) {
        std::cout << controller1.ErrorText() << std::endl;
    }
    else {
        // rpc 调用完成
        if(response1.result().errcode() == 0) {
            std::cout << "rpc register response success: " << response1.success() << std::endl;
        } 
        else {
            std::cout << "rpc register response error : " << response1.result().errmsg() << std::endl;
        }
    }
    return 0;
}