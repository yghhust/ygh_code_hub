#include "zmq_rpc_client.h"
#include "zmq_ipc_node.h"

ZmqRpcClient::ZmqRpcClient(const std::string& serverAddress)
    : node_(std::make_shared<ZmqIpcNode>(serverAddress, false)), ownsNode_(true) {}

ZmqRpcClient::ZmqRpcClient(std::shared_ptr<ZmqIpcNode> node)
    : node_(std::move(node)), ownsNode_(false) {}

ZmqRpcClient::~ZmqRpcClient() = default;

Json ZmqRpcClient::call(const std::string& service, const Json& request, int timeoutMs) {
    return node_->rpcCall(service, request, timeoutMs);
}
