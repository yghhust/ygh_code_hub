#pragma once

#include <zmq.hpp>
#include "interface/ipc_common.h"
#include <functional>
#include <string>
#include <thread>
#include <atomic>
#include <unordered_map>
#include <mutex>
#include <future>
#include <vector>

class ZmqIpcNode {
public:
    using RpcHandler = std::function<Json(const Json&)>;
    using EventHandler = std::function<void(const std::string& topic, const Json&)>;

    ZmqIpcNode(const std::string& address, bool bind);
    ~ZmqIpcNode();

    ZmqIpcNode(const ZmqIpcNode&) = delete;
    ZmqIpcNode& operator=(const ZmqIpcNode&) = delete;

    void onRpc(const std::string& service, RpcHandler handler);
    Json rpcCall(const std::string& service, const Json& request, int timeoutMs);
    void publish(const std::string& topic, const Json& data);
    void subscribe(const std::string& topic, EventHandler handler);

private:
    void workerLoop();
    void processMessage(const std::string& payload);

    zmq::context_t ctx_;
    zmq::socket_t sock_;
    std::thread worker_;
    std::atomic<bool> running_{true};

    std::mutex mutex_;
    std::unordered_map<std::string, RpcHandler> rpcHandlers_;
    std::unordered_map<std::string, std::vector<EventHandler>> eventHandlers_;

    struct PendingCall {
        std::promise<Json> promise;
        std::chrono::steady_clock::time_point deadline;
    };
    std::unordered_map<std::string, PendingCall> pendingCalls_;
    std::atomic<uint64_t> nextReqId_{0};
};
