#pragma once
#include "interface/irpc_client.h"
#include <memory>

class ZmqIpcNode;

class ZmqRpcClient : public IRpcClient {
public:
    explicit ZmqRpcClient(const std::string& serverAddress);
    explicit ZmqRpcClient(std::shared_ptr<ZmqIpcNode> node);
    ~ZmqRpcClient() override;
    Json call(const std::string& service, const Json& request, int timeoutMs = 1000) override;
private:
    std::shared_ptr<ZmqIpcNode> node_;
    bool ownsNode_;
};
