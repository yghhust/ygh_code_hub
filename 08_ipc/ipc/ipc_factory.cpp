#include "ipc_factory.h"
#include "zmq/zmq_rpc_client.h"
#include "zmq/zmq_rpc_server.h"
#include "zmq/zmq_event_bus.h"
#include <stdexcept>

std::unique_ptr<IRpcClient> IpcFactory::createRpcClient(IpcBackend backend, const std::string& address) {
    if (backend == IpcBackend::ZeroMQ) return std::make_unique<ZmqRpcClient>(address);
    throw std::runtime_error("Unsupported IPC backend");
}

std::unique_ptr<IRpcServer> IpcFactory::createRpcServer(IpcBackend backend, const std::string& address) {
    if (backend == IpcBackend::ZeroMQ) return std::make_unique<ZmqRpcServer>(address);
    throw std::runtime_error("Unsupported IPC backend");
}

std::unique_ptr<IEventBus> IpcFactory::createEventBus(IpcBackend backend, const std::string& address, bool bind) {
    if (backend == IpcBackend::ZeroMQ) return std::make_unique<ZmqEventBus>(address, bind);
    throw std::runtime_error("Unsupported IPC backend");
}
