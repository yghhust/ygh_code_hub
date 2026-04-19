// ipc/interface/ipc_common.h
#pragma once
#include <nlohmann/json.hpp>
#include <stdexcept>

using Json = nlohmann::json;

// RPC 异常类
class RpcException : public std::runtime_error {
public:
    explicit RpcException(const std::string& msg) : std::runtime_error(msg) {}
};