#include "zmq_context.h"
#include "zmq_service.h"
#include "zmq_client.h"

ZmqContext::ZmqContext() : zmqCtx_(1) {}
ZmqContext::~ZmqContext() = default;

std::unique_ptr<IService> ZmqContext::bind(const std::string& address, const std::string& servName) {
    localServName_ = servName;
    return std::make_unique<ZmqService>(zmqCtx_, address, servName);
}

std::unique_ptr<IClient> ZmqContext::createClient(const std::string& address, const std::string& targetServName) {
    return std::make_unique<ZmqClient>(zmqCtx_, address, localServName_, targetServName);
}
