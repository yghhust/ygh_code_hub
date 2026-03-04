#include "spdlogger.h"

// 使用示例
int main() {
    try {
        // 创建 spdlog 日志器
        Logger<spdlog::logger> spdlog_logger;
        spdlog_logger.init("AppLogger", 
                           false,         // 使用异步日志
                           true,          // console
                           "app.log"); // 初始日志级别
        
        // 设置日志级别
        spdlog_logger.set_level(LogLevel::DEBUG);
        
        // 记录日志
        spdlog_logger.trace("This is a trace message");
        spdlog_logger.debug("This is a debug message with value: {}", 42);
        spdlog_logger.info("Application started at {}", "test");
        spdlog_logger.warn("Low memory: {} MB free", 128);
        spdlog_logger.error("Failed to open file: {}", "/path/to/file.txt");
        spdlog_logger.critical("System shutdown initiated");
        
        // 使用工厂函数创建 fake 日志器
        //auto fake_logger = create_logger("fake");
        //fake_logger->init("FakeLogger");
        //fake_logger->set_level(LogLevel::INFO);
        //fake_logger->info("This is a fake log message: {}", 3.14);
        
        // 使用基类指针的多态行为
        ILogger* logger = &spdlog_logger;
        logger->warn("Using base class pointer");
        
        //尝试使用未特化的类型（会抛出异常）
        Logger<int> invalid_logger;
        invalid_logger.init("Invalid");
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
