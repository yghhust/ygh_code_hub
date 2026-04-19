#pragma once
#include "interface/irpc_server.h"
#include <memory>

class ZmqIpcNode;

class ZmqRpcServer : public IRpcServer {
public:
    explicit ZmqRpcServer(const std::string& bindAddress);
    explicit ZmqRpcServer(std::shared_ptr<ZmqIpcNode> node);
    ~ZmqRpcServer() override;
    void on(const std::string& service, std::function<Json(const Json&)> handler) override;
    void start() override;
    void stop() override;
private:
    std::shared_ptr<ZmqIpcNode> node_;
    bool ownsNode_;
};
