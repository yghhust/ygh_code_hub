#pragma once

#include <string>
#include <functional>
#include "ipc_common.h"

class IEventBus {
public:
    using EventCallback = std::function<void(const std::string& topic, const Json& data)>;
    virtual ~IEventBus() = default;

    // JSON 接口
    virtual void publish(const std::string& topic, const Json& data) = 0;
    virtual void subscribe(const std::string& topic, EventCallback callback) = 0;

    // 发布强类型数据
    template<typename T>
    void publish(const std::string& topic, const T& data) {
        publish(topic, Json(data));
    }

    // 订阅强类型数据
    template<typename T>
    void subscribe(const std::string& topic, std::function<void(const std::string&, const T&)> callback) {
        subscribe(topic, [callback](const std::string& t, const Json& j) {
            T obj = j.get<T>();
            callback(t, obj);
        });
    }
};
