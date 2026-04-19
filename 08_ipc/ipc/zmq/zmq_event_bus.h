#pragma once
#include "interface/ievent_bus.h"
#include <memory>

class ZmqIpcNode;

class ZmqEventBus : public IEventBus {
public:
    // bindOrConnect: true 表示绑定（发布方），false 表示连接（订阅方）
    ZmqEventBus(const std::string& address, bool bindOrConnect);
    explicit ZmqEventBus(std::shared_ptr<ZmqIpcNode> node);
    ~ZmqEventBus() override;
    void publish(const std::string& topic, const Json& data) override;
    void subscribe(const std::string& topic, EventCallback callback) override;
private:
    std::shared_ptr<ZmqIpcNode> node_;
    bool ownsNode_;
};
