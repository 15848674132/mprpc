#include "mprpccontroller.h"

MprpcController::MprpcController() 
    : m_failed(false), m_errText("")
{}

void MprpcController::Reset() {

}

bool MprpcController::Failed() const {
    return m_failed;
}

std::string MprpcController::ErrorText() const {
    return m_errText;
}

void MprpcController::SetFailed(const std::string &reason) {
    m_failed = true;
    m_errText = reason;
}

bool MprpcController::IsCanceled() const {
    return false;
}  

void MprpcController::StartCancel() {}

void MprpcController::NotifyOnCancel(google::protobuf::Closure *callback) {}