#include <iostream>
#include <string>
#include "user.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"
#include "logger.h"
/**
 * UserService原来是一个本地服务，提供了两个进程内的本地方法，Login和GetFriendsList
 */

class UserService : public fixbug::UserServiceRpc // 使用在rpc服务端（rpc服务提供者）
{
public:
    bool Login(std::string name, std::string password) {
        std::cout << "doing local service: Login" << std::endl;
        std::cout << "name:" << name << " password:" << password << std::endl;
        return false;
    }
    
    bool Register(std::string name, std::string password) {
        std::cout << "doing local service: Register" << std::endl;
        std::cout << "name:" << name << " password:" << password << std::endl;
        return true;
    }
   /**
    * 重写基类UserServiceRpc的虚函数，下面这些方法都是框架直接调用的 
    * 1. caller ===> Login(LoginRequest) ===> muduo ===> callee
    * 2. callee ===> Login(LoginRequest) ===> virtual void Login()
   */
    virtual void Login(::google::protobuf::RpcController *controller,
                       const ::fixbug::LoginRequest *request,
                       ::fixbug::LoginResponse *response,
                       ::google::protobuf::Closure *done) {
        // 1.rpc框架给业务上报了请求参数LoginRequest,应用获取相应数据做本地业务
        std::string name = request->name();
        std::string password = request->password();

        // 2.做本地业务
        bool login_result = Login(name, password);
        
        // 3.把响应写框架
        fixbug::ResultCode *resultcode = response->mutable_result();
        resultcode->set_errcode(1);
        resultcode->set_errmsg("Login do error");
        // response->set_allocated_result(resultcode);
        response->set_success(login_result);

        // 4.执行回调操作 执行相应对象的序列化和网络发送 ===> 都是由框架完成的
        done->Run();  
    }

    virtual void Register(::google::protobuf::RpcController* controller,
                       const ::fixbug::RegisterRequest* request,
                       ::fixbug::RegisterResponse* response,
                       ::google::protobuf::Closure* done) {
        // 1.rpc框架给业务上报了请求参数LoginRequest,应用获取相应数据做本地业务
        std::string name = request->name();
        std::string password = request->password();

        // 2.做本地业务
        bool register_result = Register(name, password);
        
        // 3.把响应写框架
        fixbug::ResultCode *resultcode = response->mutable_result();
        resultcode->set_errcode(0);
        resultcode->set_errmsg("");
        response->set_success(register_result);

        // 4.执行回调操作 执行相应对象的序列化和网络发送 ===> 都是由框架完成的
        done->Run();
    }
};

int main(int argc, char* argv[])
{
    LOG_INFO("first log message!");
    LOG_ERROR("%s:%s:%d", __FILE__, __FUNCTION__, __LINE__);
    // 调用框架的初始化操作
    MprpcApplication::Init(argc, argv);

    // 在框架上面发布服务
    RpcProvider provider;
    provider.NotifyService(new UserService());
    // provider.NotifyService(new ProductService());
    // ...

    // 启动一个rpc服务发布节点 Run()以后，进程进入阻塞状态，等待远程调用的rpc调用请求
    provider.Run();
    return 0;
}