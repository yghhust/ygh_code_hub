#pragma once

#include <nlohmann/json.hpp>
#include <stdexcept>

using Json = nlohmann::json;

class IpcException : public std::runtime_error {
public:
    explicit IpcException(const std::string& msg) : std::runtime_error(msg) {}
};

class RpcException : public IpcException {
public:
    explicit RpcException(const std::string& msg) : IpcException(msg) {}
};

class ConnectionException : public IpcException {
public:
    explicit ConnectionException(const std::string& msg) : IpcException(msg) {}
};
