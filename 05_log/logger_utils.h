#pragma once
#pragma once

#include <string>
#include "format_utils.h"

// 日志级别（C++17 兼容）
enum class LogLevel {
    TRACE, DEBUG, INFO, WARN, ERROR, CRITICAL
};

// 抽象日志接口
class ILogger {
public:
    virtual ~ILogger() = default;
    
    virtual void set_level(LogLevel level) = 0;
    
    template<typename... Args>
    void trace(const std::string& fmt, Args&&... args) {
        log(LogLevel::TRACE, Formatter::format(fmt, std::forward<Args>(args)...));
    }
    
    template<typename... Args>
    void debug(const std::string& fmt, Args&&... args) {
        log(LogLevel::DEBUG, Formatter::format(fmt, std::forward<Args>(args)...));
    }
    
    template<typename... Args>
    void info(const std::string& fmt, Args&&... args) {
        log(LogLevel::INFO, Formatter::format(fmt, std::forward<Args>(args)...));
    }
    
    template<typename... Args>
    void warn(const std::string& fmt, Args&&... args) {
        log(LogLevel::WARN, Formatter::format(fmt, std::forward<Args>(args)...));
    }
    
    template<typename... Args>
    void error(const std::string& fmt, Args&&... args) {
        log(LogLevel::ERROR, Formatter::format(fmt, std::forward<Args>(args)...));
    }
    
    template<typename... Args>
    void critical(const std::string& fmt, Args&&... args) {
        log(LogLevel::CRITICAL, Formatter::format(fmt, std::forward<Args>(args)...));
    }

protected:
    virtual void log(LogLevel level, const std::string& message) = 0;
};

// 主模板声明（抽象基类）
template<typename T>
class Logger : public ILogger {
public:
    // 主模板不提供实现，强制使用特化版本
    void init(const std::string& name, 
              bool async_flag = false,
              bool console_enable = true,
              const std::string& filename = "",
              LogLevel level = LogLevel::INFO) {
        throw std::runtime_error("Logger not implemented for this type");
    }
    
    void set_level(LogLevel level) override {
        throw std::runtime_error("Logger not implemented for this type");
    }
    
    // 获取底层日志器
    std::shared_ptr<T> get_logger() const {
        throw std::runtime_error("Logger not implemented for this type");
    }
protected:
    void log(LogLevel level, const std::string& message) override {
        throw std::runtime_error("Logger not implemented for this type");
    }
};

template<typename T>
using LoggerPtr = std::shared_ptr<Logger<T>>;
