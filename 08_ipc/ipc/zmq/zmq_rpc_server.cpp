#include "zmq_rpc_server.h"
#include "zmq_ipc_node.h"

ZmqRpcServer::ZmqRpcServer(const std::string& bindAddress)
    : node_(std::make_shared<ZmqIpcNode>(bindAddress, true)), ownsNode_(true) {}

ZmqRpcServer::ZmqRpcServer(std::shared_ptr<ZmqIpcNode> node)
    : node_(std::move(node)), ownsNode_(false) {}

ZmqRpcServer::~ZmqRpcServer() { stop(); }

void ZmqRpcServer::on(const std::string& service, std::function<Json(const Json&)> handler) {
    node_->onRpc(service, std::move(handler));
}

void ZmqRpcServer::start() {}
void ZmqRpcServer::stop() { node_.reset(); }
