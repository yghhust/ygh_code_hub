#pragma once
#include "interface/irpc_client.h"
#include "interface/irpc_server.h"
#include "interface/ievent_bus.h"
#include <memory>
#include <string>

class ZmqIpcNode;

class ZmqContext {
public:
    static ZmqContext createServer(const std::string& address);
    static ZmqContext createClient(const std::string& address);

    ~ZmqContext();

    ZmqContext(const ZmqContext&) = delete;
    ZmqContext& operator=(const ZmqContext&) = delete;
    ZmqContext(ZmqContext&&) = default;
    ZmqContext& operator=(ZmqContext&&) = default;

    std::unique_ptr<IRpcClient> createRpcClient();
    std::unique_ptr<IRpcServer> createRpcServer();
    std::unique_ptr<IEventBus> createEventBus();

private:
    ZmqContext(const std::string& address, bool bind);
    std::shared_ptr<ZmqIpcNode> node_;
};
