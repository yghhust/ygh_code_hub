// ipc/interface/irpc_client.h
#pragma once

#include <string>
#include <functional>
#include "ipc_common.h"

class IRpcClient {
public:
    virtual ~IRpcClient() = default;

    // 原始 JSON 接口（保留）
    virtual Json call(const std::string& service, const Json& request, int timeoutMs = 1000) = 0;

    // 强类型模板接口（带错误检查）
    template<typename Response, typename Request>
    Response call(const std::string& service, const Request& req, int timeoutMs = 1000) {
        Json reqJson = req;
        Json respJson = call(service, reqJson, timeoutMs);

        // 检查是否为错误响应
        if (respJson.contains("error")) {
            throw RpcException("RPC call to '" + service + "' failed: " + respJson["error"].get<std::string>());
        }

        return respJson.get<Response>();
    }

    // 无请求参数版本（带错误检查）
    template<typename Response>
    Response call(const std::string& service, int timeoutMs = 1000) {
        Json reqJson = Json::object();
        Json respJson = call(service, reqJson, timeoutMs);

        if (respJson.contains("error")) {
            throw RpcException("RPC call to '" + service + "' failed: " + respJson["error"].get<std::string>());
        }

        return respJson.get<Response>();
    }
};
