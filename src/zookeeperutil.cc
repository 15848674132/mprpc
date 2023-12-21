#include "zookeeperutil.h"
#include "mprpcapplication.h"
#include "logger.h"
// 全局的watcher观察器 zkserver给zkclient的通知
static void global_watcher(zhandle_t *zh, int type, int state, 
                    const char *path, void *watchCtx) {
    if(type == ZOO_SESSION_EVENT) { // 回调的消息类型是和会话相关的消息类型
        if(state == ZOO_CONNECTED_STATE) { // zkclient和zkserver连接成功
            sem_t *sem = (sem_t*)zoo_get_context(zh);
            sem_post(sem);
        }
    }
}

ZkClient::ZkClient() : m_zhandle(nullptr) {}

ZkClient::~ZkClient() {
    if(m_zhandle != nullptr)
        zookeeper_close(m_zhandle);
}

// zkclient 连接zkserver
void ZkClient::Start() {
    std::string ip = MprpcApplication::GetInstance()->Getconfig().Load("zookeeperip");
    std::string port = MprpcApplication::GetInstance()->Getconfig().Load("zookeeperport");
    std::string connstr = ip + ":" + port;
    /**
     * zookeeper_mt: 多线程版本
     * zookeeper的API客户端提供了三个线程
     * API调用线程
     * 网络I/O线程 pthread_create + poll 异步的连接过程
     * watcher回调线程 pthread_create
    */
    
    m_zhandle = zookeeper_init(connstr.c_str(), global_watcher, 300000, nullptr, nullptr, 0);
    if(nullptr == m_zhandle) {
        LOG_ERROR("%s:%s:%d ===> zookeeper_init error!", __FILE__, __FUNCTION__, __LINE__);
        exit(EXIT_FAILURE);
    }
    LOG_INFO("%s:%s:%d ===> zookeeper_init success!", __FILE__, __FUNCTION__, __LINE__);

    sem_t sem;
    sem_init(&sem, 0, 0);
    zoo_set_context(m_zhandle, &sem);
    sem_wait(&sem);
    LOG_INFO("%s:%s:%d ===> zkclient connect zkserver success!", __FILE__, __FUNCTION__, __LINE__);
}

// 在zkserver上根据指定的path创建znode节点
void ZkClient::Create(const char *path, const char *data, int datalen, int state) {
    char path_buffer[128];
    int bufferlen = sizeof path_buffer;
    // 先判断path表示的znode节点是否存在，如果存在，就不再重复创建了
    int flag = zoo_exists(m_zhandle, path, 0, nullptr);
    if(ZNONODE == flag) { // 表示path的znoke节点不存在
        flag = zoo_create(m_zhandle, path, data, datalen,
                            &ZOO_OPEN_ACL_UNSAFE, state, path_buffer, bufferlen);
        if(ZOK == flag) {
            LOG_INFO("%s:%s:%d ===> znode create success... path:%s", __FILE__, __FUNCTION__, __LINE__, path);
        }
        else {
            LOG_ERROR("%s:%s:%d ===> flag:%d znode create success... path:%s", __FILE__, __FUNCTION__, __LINE__, flag, path);
            exit(EXIT_FAILURE);
        }
    }
}
// 根据参数指定的znode节点路径，获取znode节点的值
std::string ZkClient::GetData(const char *path) {
    char buffer[64];
    int bufferlen = sizeof buffer;
    int flag = zoo_get(m_zhandle, path, 0, buffer, &bufferlen, nullptr);
    if(ZOK != flag) {
        LOG_ERROR("%s:%s:%d ===> flag:%d get znode error... path:%s", __FILE__, __FUNCTION__, __LINE__, flag, path);
        return ""; 
    }
    else {
        return buffer;
    }
}