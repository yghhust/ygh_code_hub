#pragma once

#include "ipc_common.h"
#include <functional>
#include <string>

class IClient {
public:
    using EventCallback = std::function<void(const std::string& topic, const Json& data)>;
    virtual ~IClient() = default;

    virtual Json call(const std::string& service, const Json& request, int timeoutMs = 3000) = 0;
    virtual void subscribe(const std::string& topic, EventCallback callback) = 0;
    virtual void unsubscribe(const std::string& topic) = 0;

    template<typename Response, typename Request>
    Response call(const std::string& service, const Request& req, int timeoutMs = 3000) {
        Json reqJson = req;
        Json respJson = call(service, reqJson, timeoutMs);
        if (respJson.contains("error")) {
            throw RpcException("RPC call to '" + service + "' failed: " +
                               respJson["error"].get<std::string>());
        }
        return respJson.get<Response>();
    }

    template<typename T>
    void subscribe(const std::string& topic,
                   std::function<void(const std::string&, const T&)> callback) {
        subscribe(topic, [callback](const std::string& t, const Json& j) {
            T obj = j.get<T>();
            callback(t, obj);
        });
    }
};
