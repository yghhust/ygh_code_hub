#pragma once

#include <memory>
#include <string>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <mutex>
#include <sstream>

namespace ipc {

enum class LogLevel { Debug, Info, Warn, Error };

class ILogger {
public:
    virtual ~ILogger() = default;
    virtual void log(LogLevel level, const std::string& message) = 0;
};

class ConsoleLogger : public ILogger {
public:
    void log(LogLevel level, const std::string& message) override {
        static std::mutex mutex;
        std::lock_guard<std::mutex> lock(mutex);
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        std::clog << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S")
                  << '.' << std::setfill('0') << std::setw(3) << ms.count()
                  << " [" << levelToString(level) << "] " << message << std::endl;
    }
private:
    static const char* levelToString(LogLevel level) {
        switch (level) {
            case LogLevel::Debug: return "DEBUG";
            case LogLevel::Info:  return "INFO ";
            case LogLevel::Warn:  return "WARN ";
            case LogLevel::Error: return "ERROR";
            default: return "?????";
        }
    }
};

class LoggerManager {
public:
    static LoggerManager& instance() {
        static LoggerManager mgr;
        return mgr;
    }
    void setLogger(std::shared_ptr<ILogger> logger) {
        std::lock_guard<std::mutex> lock(mutex_);
        logger_ = std::move(logger);
    }
    ILogger* getLogger() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!logger_) {
            static auto defaultLogger = std::make_shared<ConsoleLogger>();
            logger_ = defaultLogger;
        }
        return logger_.get();
    }
private:
    LoggerManager() = default;
    std::shared_ptr<ILogger> logger_;
    std::mutex mutex_;
};

// 上下文日志器：输出 c_ctrx 结构化日志
class ContextLogger {
public:
    explicit ContextLogger(std::string component) : component_(std::move(component)) {}

    void log(LogLevel level, const std::string& type, const std::string& from, const std::string& to,
             const std::string& payload = "") {
        std::ostringstream oss;
        oss << R"("c_ctrx": {"type":")" << type << R"(", "from":")" << from << R"(", "to":")" << to << R"("})";
        if (!payload.empty()) oss << R"(, "payload":)" << payload;
        auto* logger = LoggerManager::instance().getLogger();
        if (logger) {
            logger->log(level, "[" + component_ + "] " + oss.str());
        }
    }

    void info(const std::string& type, const std::string& from, const std::string& to, const std::string& payload = "") {
        log(LogLevel::Info, type, from, to, payload);
    }
    void debug(const std::string& type, const std::string& from, const std::string& to, const std::string& payload = "") {
        log(LogLevel::Debug, type, from, to, payload);
    }
    void warn(const std::string& type, const std::string& from, const std::string& to, const std::string& payload = "") {
        log(LogLevel::Warn, type, from, to, payload);
    }
    void error(const std::string& type, const std::string& from, const std::string& to, const std::string& error) {
        std::ostringstream oss;
        oss << R"("c_ctrx": {"type":")" << type << R"(", "from":")" << from << R"(", "to":")" << to << R"(", "error":")" << error << R"("})";
        auto* logger = LoggerManager::instance().getLogger();
        if (logger) {
            logger->log(LogLevel::Error, "[" + component_ + "] " + oss.str());
        }
    }

private:
    std::string component_;
};

} // namespace ipc
