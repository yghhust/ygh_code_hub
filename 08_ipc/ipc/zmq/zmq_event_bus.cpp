#include "zmq_event_bus.h"
#include "zmq_ipc_node.h"

ZmqEventBus::ZmqEventBus(const std::string& address, bool bindOrConnect)
    : node_(std::make_shared<ZmqIpcNode>(address, bindOrConnect)), ownsNode_(true) {}

ZmqEventBus::ZmqEventBus(std::shared_ptr<ZmqIpcNode> node)
    : node_(std::move(node)), ownsNode_(false) {}

ZmqEventBus::~ZmqEventBus() = default;

void ZmqEventBus::publish(const std::string& topic, const Json& data) {
    node_->publish(topic, data);
}

void ZmqEventBus::subscribe(const std::string& topic, EventCallback callback) {
    node_->subscribe(topic, std::move(callback));
}
