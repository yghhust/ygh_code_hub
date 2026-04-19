#pragma once

#include "interface/ipc_context.h"
#include <zmq.hpp>
#include <string>

class ZmqContext : public IIpcContext {
public:
    ZmqContext();
    ~ZmqContext() override;

    std::unique_ptr<IService> bind(const std::string& address, const std::string& servName) override;
    std::unique_ptr<IClient> createClient(const std::string& address, const std::string& targetServName) override;

private:
    zmq::context_t zmqCtx_;
    std::string localServName_;   // 由 bind 设置的本地节点名
};
