#pragma once

#include <string>
#include <functional>
#include "ipc_common.h"

class IRpcServer {
public:
    virtual ~IRpcServer() = default;
    virtual void on(const std::string& service, std::function<Json(const Json&)> handler) = 0;

    // 强类型绑定，自动序列化/反序列化
    template<typename Response, typename Request>
    void on(const std::string& service, std::function<Response(const Request&)> handler) {
        on(service, [handler](const Json& reqJson) -> Json {
            Request req = reqJson.get<Request>();   // from_json
            Response resp = handler(req);
            return Json(resp);                      // to_json
        });
    }

    // 无请求参数的版本
    template<typename Response>
    void on(const std::string& service, std::function<Response()> handler) {
        on(service, [handler](const Json&) -> Json {
            Response resp = handler();
            return Json(resp);
        });
    }

    virtual void start() = 0;
    virtual void stop() = 0;
};
