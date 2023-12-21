#ifndef MPRPCCHANNEL_H
#define MPRPCCHANNEL_H

#include <google/protobuf/service.h>

class MprpcChannel : public google::protobuf::RpcChannel {
public:
    virtual void CallMethod(const google::protobuf::MethodDescriptor*,
                            google::protobuf::RpcController*, const google::protobuf::Message*,
                            google::protobuf::Message*, google::protobuf::Closure*);
};



#endif