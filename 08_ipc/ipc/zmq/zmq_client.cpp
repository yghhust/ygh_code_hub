#include "zmq_client.h"
#include <iostream>
#include <sstream>
#include <iomanip>

ZmqClient::ZmqClient(zmq::context_t& ctx, const std::string& connectAddr,
                     const std::string& clientName, const std::string& targetServName)
    : sock_(ctx, ZMQ_DEALER), clientName_(clientName), targetServName_(targetServName), log_("Client") {
    try {
        sock_.connect(connectAddr);
        log_.info("connect", clientName_, targetServName_);
    } catch (const zmq::error_t& e) {
        log_.error("connect_error", clientName_, targetServName_, e.what());
        throw ConnectionException("Failed to connect to " + connectAddr + ": " + e.what());
    }
    worker_ = std::thread(&ZmqClient::workerLoop, this);
}

ZmqClient::~ZmqClient() {
    for (auto& pair : subscribers_) {
        try { sendUnsubscribe(pair.first); } catch (...) {}
    }

    running_ = false;
    int linger = 0;
    sock_.set(zmq::sockopt::linger, linger);
    sock_.close();

    if (worker_.joinable()) worker_.join();
    log_.info("destroyed", clientName_, "");
}

Json ZmqClient::call(const std::string& service, const Json& request, int timeoutMs) {
    std::string reqId = std::to_string(nextReqId_++);
    Json req = {
        {"type", "rpc_req"},
        {"service", service},
        {"id", reqId},
        {"payload", request},
        {"from", clientName_}
    };
    std::string reqStr = req.dump();

    log_.info("rpc_req_" + service, clientName_, targetServName_, request.dump());

    std::promise<Json> promise;
    std::future<Json> future = promise.get_future();
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingCalls_[reqId] = std::move(promise);
    }
    sock_.send(zmq::buffer(reqStr), zmq::send_flags::none);

    auto status = future.wait_for(std::chrono::milliseconds(timeoutMs));
    if (status == std::future_status::ready) {
        Json resp = future.get();
        log_.info("rpc_rsp_" + service, targetServName_, clientName_, resp["payload"].dump());
        return resp;
    } else {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingCalls_.erase(reqId);
        log_.error("rpc_timeout_" + service, clientName_, targetServName_, std::to_string(timeoutMs) + "ms");
        throw RpcException("RPC call timeout");
    }
}

void ZmqClient::subscribe(const std::string& topic, EventCallback callback) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        subscribers_[topic] = std::move(callback);
    }
    sendSubscribe(topic);
    log_.info("sub_" + topic, clientName_, targetServName_);
}

void ZmqClient::unsubscribe(const std::string& topic) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        subscribers_.erase(topic);
    }
    sendUnsubscribe(topic);
    log_.info("unsub_" + topic, clientName_, targetServName_);
}

void ZmqClient::sendSubscribe(const std::string& topic) {
    Json msg = {{"type", "subscribe"}, {"topic", topic}, {"name", clientName_}};
    sock_.send(zmq::buffer(msg.dump()), zmq::send_flags::none);
}

void ZmqClient::sendUnsubscribe(const std::string& topic) {
    Json msg = {{"type", "unsubscribe"}, {"topic", topic}, {"name", clientName_}};
    sock_.send(zmq::buffer(msg.dump()), zmq::send_flags::none);
}

void ZmqClient::workerLoop() {
    while (running_) {
        zmq::message_t msg;
        auto res = sock_.recv(msg, zmq::recv_flags::dontwait);
        if (res.has_value()) {
            std::string payload(static_cast<char*>(msg.data()), msg.size());
            try {
                Json j = Json::parse(payload);
                std::string type = j.value("type", "");
                if (type == "rpc_resp") {
                    std::string id = j.value("id", "");
                    Json respPayload = j["payload"];
                    std::lock_guard<std::mutex> lock(mutex_);
                    auto it = pendingCalls_.find(id);
                    if (it != pendingCalls_.end()) {
                        it->second.set_value(respPayload);
                        pendingCalls_.erase(it);
                    }
                } else if (type == "event") {
                    std::string topic = j["topic"];
                    Json data = j["payload"];
                    log_.info("evt_" + topic, targetServName_, clientName_, data.dump());
                    EventCallback cb;
                    {
                        std::lock_guard<std::mutex> lock(mutex_);
                        auto it = subscribers_.find(topic);
                        if (it != subscribers_.end()) cb = it->second;
                    }
                    if (cb) cb(topic, data);
                }
            } catch (const std::exception& e) {
                log_.error("parse_error", "Service", clientName_, e.what());
            }
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}
