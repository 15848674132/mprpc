#ifndef MPRPCCONTROLLER_H
#define MPRPCCONTROLLER_H

#include <google/protobuf/service.h>

class MprpcController : public google::protobuf::RpcController {
public:
    MprpcController();
    virtual ~MprpcController() = default;
    
    virtual void Reset();

    virtual bool Failed() const;

    virtual std::string ErrorText() const;

    virtual void StartCancel();

    virtual void SetFailed(const std::string& reason);

    virtual bool IsCanceled() const;

    virtual void NotifyOnCancel(google::protobuf::Closure* callback);
private:
    bool m_failed;
    std::string m_errText;
};

#endif