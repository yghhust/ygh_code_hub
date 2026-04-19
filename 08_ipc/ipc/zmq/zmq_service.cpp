#include "zmq_service.h"
#include <sstream>
#include <iomanip>

std::string ZmqService::toHex(const std::string& str) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (unsigned char c : str) oss << std::setw(2) << static_cast<int>(c);
    return oss.str();
}

ZmqService::ZmqService(zmq::context_t& ctx, const std::string& bindAddr, const std::string& servName)
    : sock_(ctx, ZMQ_ROUTER), servName_(servName), log_("Service") {
    try {
        sock_.bind(bindAddr);
        log_.info("bind", servName_, "");
    } catch (const zmq::error_t& e) {
        log_.error("bind_error", servName_, bindAddr, e.what());
        throw ConnectionException("Failed to bind to " + bindAddr + ": " + e.what());
    }
}

ZmqService::~ZmqService() { stop(); }

void ZmqService::on(const std::string& service, std::function<Json(const Json&)> handler) {
    std::lock_guard<std::mutex> lock(mutex_);
    rpcHandlers_[service] = std::move(handler);
    log_.info("reg_handler", servName_, "");
}

void ZmqService::publish(const std::string& topic, const Json& data) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = subscribers_.find(topic);
    if (it == subscribers_.end()) return;

    Json msg = {{"type", "event"}, {"topic", topic}, {"payload", data}};
    std::string msgStr = msg.dump();

    for (const auto& id : it->second) {
        std::string clientName = identityToClient_.count(id) ? identityToClient_[id] : toHex(id);
        sock_.send(zmq::buffer(id), zmq::send_flags::sndmore);
        sock_.send(zmq::buffer(msgStr), zmq::send_flags::none);
        log_.info("evt_" + topic, servName_, clientName, data.dump());
    }
}

void ZmqService::start() {
    if (running_.exchange(true)) return;
    worker_ = std::thread(&ZmqService::workerLoop, this);
    log_.info("started", servName_, "");
}

void ZmqService::stop() {
    if (!running_.exchange(false)) return;

    int linger = 0;
    sock_.set(zmq::sockopt::linger, linger);
    sock_.close();

    if (worker_.joinable()) worker_.join();
    log_.info("stopped", servName_, "");
}

void ZmqService::workerLoop() {
    while (running_) {
        zmq::message_t identity, payload;
        if (!sock_.recv(identity, zmq::recv_flags::dontwait)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }
        if (!sock_.recv(payload, zmq::recv_flags::none)) continue;

        std::string id(static_cast<char*>(identity.data()), identity.size());
        std::string msg(static_cast<char*>(payload.data()), payload.size());

        try {
            Json req = Json::parse(msg);
            std::string type = req.value("type", "");

            if (type == "rpc_req") {
                std::string reqId = req.value("id", "");
                std::string service = req["service"];
                Json reqPayload = req["payload"];
                std::string clientName = req.value("from", identityToClient_.count(id) ? identityToClient_[id] : toHex(id));

                log_.info("rpc_req_" + service, clientName, servName_, reqPayload.dump());

                std::function<Json(const Json&)> handler;
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    auto it = rpcHandlers_.find(service);
                    if (it != rpcHandlers_.end()) handler = it->second;
                }

                Json respPayload;
                if (handler) {
                    try {
                        respPayload = handler(reqPayload);
                    } catch (const std::exception& e) {
                        respPayload = {{"error", e.what()}};
                        log_.error("rpc_error_" + service, clientName, servName_, e.what());
                    }
                } else {
                    respPayload = {{"error", "unknown service"}};
                    log_.error("rpc_unknown_" + service, clientName, servName_, "unknown service");
                }

                Json resp = {{"type", "rpc_resp"}, {"id", reqId}, {"payload", respPayload}};
                std::string respStr = resp.dump();
                sock_.send(zmq::buffer(id), zmq::send_flags::sndmore);
                sock_.send(zmq::buffer(respStr), zmq::send_flags::none);

                log_.info("rpc_rsp_" + service, servName_, clientName, respPayload.dump());
            }
            else if (type == "subscribe") {
                std::string topic = req["topic"];
                std::string clientName = req.value("name", toHex(id));
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    subscribers_[topic].insert(id);
                    identityToClient_[id] = clientName;
                }
                log_.info("sub_" + topic, clientName, servName_);
            }
            else if (type == "unsubscribe") {
                std::string topic = req["topic"];
                std::string clientName = identityToClient_.count(id) ? identityToClient_[id] : toHex(id);
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    auto it = subscribers_.find(topic);
                    if (it != subscribers_.end()) {
                        it->second.erase(id);
                        if (it->second.empty()) subscribers_.erase(it);
                    }
                    identityToClient_.erase(id);
                }
                log_.info("unsub_" + topic, clientName, servName_);
            }
        } catch (const std::exception& e) {
            log_.error("parse_error", "Service", toHex(id), e.what());
        }
    }
}
