#include "zmq_ipc_node.h"
#include <iostream>
#include <chrono>

ZmqIpcNode::ZmqIpcNode(const std::string& address, bool bind)
    : ctx_(1), sock_(ctx_, ZMQ_DEALER)
{
    int linger = 0;
    sock_.set(zmq::sockopt::linger, linger);
    if (bind) sock_.bind(address);
    else sock_.connect(address);
    worker_ = std::thread(&ZmqIpcNode::workerLoop, this);
}

ZmqIpcNode::~ZmqIpcNode() {
    running_ = false;
    if (worker_.joinable()) worker_.join();
    sock_.close();
    ctx_.close();
}

void ZmqIpcNode::workerLoop() {
    while (running_) {
        zmq::message_t msg;
        auto res = sock_.recv(msg, zmq::recv_flags::dontwait);
        if (res.has_value()) {
            std::string payload(static_cast<char*>(msg.data()), msg.size());
            processMessage(payload);
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

void ZmqIpcNode::processMessage(const std::string& payload) {
    try {
        Json j = Json::parse(payload);
        std::string type = j.value("type", "");

        if (type == "rpc_req") {
            std::string service = j["service"];
            std::string reqId = j["id"];
            Json reqPayload = j["payload"];

            Json respPayload;
            {
                std::lock_guard<std::mutex> lock(mutex_);
                auto it = rpcHandlers_.find(service);
                if (it != rpcHandlers_.end()) {
                    respPayload = it->second(reqPayload);
                } else {
                    respPayload = {{"error", "unknown service"}};
                }
            }

            Json resp = {{"type", "rpc_resp"}, {"id", reqId}, {"payload", respPayload}};
            std::string respStr = resp.dump();
            sock_.send(zmq::buffer(respStr), zmq::send_flags::none);
        }
        else if (type == "rpc_resp") {
            std::string reqId = j["id"];
            Json respPayload = j["payload"];
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = pendingCalls_.find(reqId);
            if (it != pendingCalls_.end()) {
                it->second.promise.set_value(respPayload);
                pendingCalls_.erase(it);
            }
        }
        else if (type == "event") {
            std::string topic = j["topic"];
            Json data = j["payload"];
            std::lock_guard<std::mutex> lock(mutex_);
            for (auto& [pattern, handlers] : eventHandlers_) {
                if (topic.find(pattern) == 0) {
                    for (auto& h : handlers) {
                        h(topic, data);
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "ZmqIpcNode error: " << e.what() << std::endl;
    }
}

void ZmqIpcNode::onRpc(const std::string& service, RpcHandler handler) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (rpcHandlers_.count(service) > 0) {
        std::cerr << "Warning: Overwriting existing RPC handler for service '" << service << "'" << std::endl;
    }
    rpcHandlers_[service] = std::move(handler);
}

Json ZmqIpcNode::rpcCall(const std::string& service, const Json& request, int timeoutMs) {
    std::string reqId = std::to_string(nextReqId_++);
    Json req = {{"type", "rpc_req"}, {"service", service}, {"id", reqId}, {"payload", request}};
    std::string reqStr = req.dump();

    std::promise<Json> promise;
    std::future<Json> future = promise.get_future();
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingCalls_[reqId] = {std::move(promise), std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs)};
    }

    sock_.send(zmq::buffer(reqStr), zmq::send_flags::none);

    auto status = future.wait_for(std::chrono::milliseconds(timeoutMs));
    if (status == std::future_status::ready) {
        return future.get();
    } else {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingCalls_.erase(reqId);
        return {{"error", "timeout"}};
    }
}

void ZmqIpcNode::publish(const std::string& topic, const Json& data) {
    Json msg = {{"type", "event"}, {"topic", topic}, {"payload", data}};
    std::string msgStr = msg.dump();
    sock_.send(zmq::buffer(msgStr), zmq::send_flags::none);
}

void ZmqIpcNode::subscribe(const std::string& topic, EventHandler handler) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (eventHandlers_.count(topic) > 0) {
        std::cerr << "Warning: Overwriting existing event handlers for topic '" << topic << "'" << std::endl;
    }
    eventHandlers_[topic].push_back(std::move(handler));
}
