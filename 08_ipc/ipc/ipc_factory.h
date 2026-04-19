#pragma once
#include "interface/irpc_client.h"
#include "interface/irpc_server.h"
#include "interface/ievent_bus.h"
#include <memory>
#include <string>

enum class IpcBackend {
    ZeroMQ,
    // 未来扩展：gRPC, UDP, etc.
};

class IpcFactory {
public:
    static std::unique_ptr<IRpcClient> createRpcClient(IpcBackend backend, const std::string& address);
    static std::unique_ptr<IRpcServer> createRpcServer(IpcBackend backend, const std::string& address);
    static std::unique_ptr<IEventBus> createEventBus(IpcBackend backend, const std::string& address, bool bind);
};
