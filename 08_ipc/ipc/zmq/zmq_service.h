#pragma once

#include "interface/ipc_service.h"
#include "interface/ipc_logger.h"
#include <zmq.hpp>
#include <thread>
#include <atomic>
#include <unordered_map>
#include <unordered_set>
#include <mutex>

class ZmqService : public IService {
public:
    ZmqService(zmq::context_t& ctx, const std::string& bindAddr, const std::string& servName);
    ~ZmqService() override;

    void on(const std::string& service, std::function<Json(const Json&)> handler) override;
    void publish(const std::string& topic, const Json& data) override;
    void start() override;
    void stop() override;

private:
    void workerLoop();
    std::string toHex(const std::string& str);

    zmq::socket_t sock_;
    std::thread worker_;
    std::atomic<bool> running_{false};
    std::string servName_;
    ipc::ContextLogger log_;

    std::unordered_map<std::string, std::function<Json(const Json&)>> rpcHandlers_;
    std::unordered_map<std::string, std::unordered_set<std::string>> subscribers_;
    std::unordered_map<std::string, std::string> identityToClient_;
    std::mutex mutex_;
};
