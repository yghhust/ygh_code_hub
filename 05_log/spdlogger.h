#pragma once

#include "logger_utils.h"
#include "spdlog/spdlog.h"
#include "spdlog/async_logger.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
     
// spdlog 特化版本(依赖spdlog库)
template<>
class Logger<spdlog::logger> : public ILogger {
public:
    Logger() = default;
   
public:    
    void init(const std::string& name, 
              bool async_flag = false,
              bool console_enable = true,
              const std::string& filename = "",
              LogLevel level = LogLevel::INFO) {
		std::vector<spdlog::sink_ptr> sinks;
		if(console_enable) {
			auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
			console_sink->set_level(spdlog::level::info);
			sinks.push_back(console_sink);
		}
		if(!filename.empty()) {
			auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("run.log", true);
			file_sink->set_level(spdlog::level::info);
			sinks.push_back(file_sink);
		}
        init(name, sinks, async_flag);
        set_level(level);
    }
    
	void init(std::string name, 
	          std::vector<spdlog::sink_ptr> sinks = {}, 
	          bool async_flag = false, 
	          spdlog::level::level_enum level = spdlog::level::info) {
        std::lock_guard<std::mutex> lock(mutex_);
        if(logger_) {
            error("logger#{} init again!\n", name);
            return;
        }
        if(async_flag) {        
        	logger_ = std::make_shared<spdlog::async_logger>(
		        name, sinks.begin(), sinks.end(),
		        std::make_shared<spdlog::details::thread_pool>(4,4),
		        spdlog::async_overflow_policy::block);
     	} else {
     		logger_ = std::make_shared<spdlog::logger>(
                name, sinks.begin(), sinks.end());
        }
     
		logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [thread %t] %v");
		//logger_->set_level(should_level_);
		spdlog::register_logger(logger_);
    }
    
    void set_level(LogLevel level) override {
    	should_level_ = level;
    }

    std::shared_ptr<spdlog::logger> get_logger() const {
        return logger_;
    }
public:  
    void log(LogLevel level, const std::string& message) override {
    	if((!logger_) || (level < should_level_)) return;
 		switch (level) {
            case LogLevel::TRACE:    return logger_->trace(message);
            case LogLevel::DEBUG:    return logger_->debug(message);
            case LogLevel::INFO:     return logger_->info(message);
            case LogLevel::WARN:     return logger_->warn(message);
            case LogLevel::ERROR:    return logger_->error(message);
            case LogLevel::CRITICAL: return logger_->critical(message);
            default: return logger_->info(message);
        }
    }

protected:
    std::mutex mutex_;
    LogLevel should_level_ = LogLevel::INFO;
    std::shared_ptr<spdlog::logger> logger_; 
};

using SpdLoggerPtr = std::shared_ptr<Logger<spdlog::logger>>;

// 全局访问点
#define LOG_TRACE(...) AutoRegister::instance().getInstance<SpdLogger>("RunLogger")->trace(__VA_ARGS__)
#define LOG_DEBUG(...) AutoRegister::instance().getInstance<SpdLogger>("RunLogger")->debug(__VA_ARGS__)
#define LOG_INFO(...)  AutoRegister::instance().getInstance<SpdLogger>("RunLogger")->info(__VA_ARGS__)
#define LOG_WARN(...)  AutoRegister::instance().getInstance<SpdLogger>("RunLogger")->warn(__VA_ARGS__)
#define LOG_ERROR(...) AutoRegister::instance().getInstance<SpdLogger>("RunLogger")->error(__VA_ARGS__)
#define LOG_CRITICAL(...) AutoRegister::instance().getInstance<SpdLogger>("RunLogger")->critical(__VA_ARGS__)

