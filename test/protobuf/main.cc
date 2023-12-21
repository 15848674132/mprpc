#include <iostream>
#include <string>
#include "test.pb.h"

using namespace std;
using namespace fixbug;


int main() {

    LoginRequest req;
    req.set_name("zhang san");
    req.set_pwd("123456");
    string send_str;
    // 序列化
    if(req.SerializeToString(&send_str)) {
        cout << send_str << endl;
    }

    // 反序列化
    LoginRequest req1;
    if(req1.ParseFromString(send_str)) {
        cout << req1.name() << endl;
        cout << req1.pwd() << endl;
    }
    


    return 0;
}