#pragma once

#include "ipc_common.h"
#include <functional>
#include <string>

class IService {
public:
    virtual ~IService() = default;

    virtual void on(const std::string& service, std::function<Json(const Json&)> handler) = 0;
    virtual void publish(const std::string& topic, const Json& data) = 0;
    virtual void start() = 0;
    virtual void stop() = 0;

    template<typename Response, typename Request>
    void on(const std::string& service, std::function<Response(const Request&)> handler) {
        on(service, [handler](const Json& reqJson) -> Json {
            try {
                Request req = reqJson.get<Request>();
                Response resp = handler(req);
                return Json(resp);
            } catch (const IpcException&) {
                throw;
            } catch (const std::exception& e) {
                return Json{{"error", e.what()}};
            } catch (...) {
                return Json{{"error", "Unknown handler error"}};
            }
        });
    }

    template<typename T>
    void publish(const std::string& topic, const T& data) {
        publish(topic, Json(data));
    }
};
