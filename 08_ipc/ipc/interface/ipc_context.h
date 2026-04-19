#pragma once

#include "ipc_service.h"
#include "ipc_client.h"
#include <memory>
#include <string>

enum class IpcBackend { ZeroMQ };

class IIpcContext {
public:
    virtual ~IIpcContext() = default;

    virtual std::unique_ptr<IService> bind(const std::string& address, const std::string& servName) = 0;

    /**
     * @brief 连接到远程服务端，创建客户端
     * @param address 目标服务端的连接地址
     * @param targetServName 目标服务端的节点名称（日志中的 to 字段）
     */
    virtual std::unique_ptr<IClient> createClient(const std::string& address,
                                                  const std::string& targetServName) = 0;
};

std::unique_ptr<IIpcContext> createIpcContext(IpcBackend backend);
