#pragma once

#include "interface/ipc_client.h"
#include "interface/ipc_logger.h"
#include <zmq.hpp>
#include <thread>
#include <atomic>
#include <mutex>
#include <unordered_map>
#include <future>

class ZmqClient : public IClient {
public:
    ZmqClient(zmq::context_t& ctx, const std::string& connectAddr,
              const std::string& clientName, const std::string& targetServName);
    ~ZmqClient() override;

    Json call(const std::string& service, const Json& request, int timeoutMs) override;
    void subscribe(const std::string& topic, EventCallback callback) override;
    void unsubscribe(const std::string& topic) override;

private:
    void workerLoop();
    void sendSubscribe(const std::string& topic);
    void sendUnsubscribe(const std::string& topic);

    zmq::socket_t sock_;
    std::thread worker_;
    std::atomic<bool> running_{true};
    std::string clientName_;
    std::string targetServName_;
    ipc::ContextLogger log_;
    std::mutex mutex_;
    std::unordered_map<std::string, EventCallback> subscribers_;
    std::unordered_map<std::string, std::promise<Json>> pendingCalls_;
    std::atomic<uint64_t> nextReqId_{0};
};
