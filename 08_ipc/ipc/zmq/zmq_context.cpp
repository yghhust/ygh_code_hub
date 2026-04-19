#include "zmq_context.h"
#include "zmq_ipc_node.h"
#include "zmq_rpc_client.h"
#include "zmq_rpc_server.h"
#include "zmq_event_bus.h"

ZmqContext ZmqContext::createServer(const std::string& address) {
    return ZmqContext(address, true);
}

ZmqContext ZmqContext::createClient(const std::string& address) {
    return ZmqContext(address, false);
}

ZmqContext::ZmqContext(const std::string& address, bool bind)
    : node_(std::make_shared<ZmqIpcNode>(address, bind)) {}

ZmqContext::~ZmqContext() = default;

std::unique_ptr<IRpcClient> ZmqContext::createRpcClient() {
    return std::make_unique<ZmqRpcClient>(node_);
}

std::unique_ptr<IRpcServer> ZmqContext::createRpcServer() {
    return std::make_unique<ZmqRpcServer>(node_);
}

std::unique_ptr<IEventBus> ZmqContext::createEventBus() {
    return std::make_unique<ZmqEventBus>(node_);
}
