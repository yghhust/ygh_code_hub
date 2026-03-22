#include "config.h"
#include <iostream>
#include <map>

// 定义配置结构体
struct LoggingConfig {
    std::string level = "INFO";
    std::string file = "app.log";
};

struct ServerConfig {
    int port = 8080;
    bool enable_ssl = false;
    // 假设有一个枚举 Mode，需要转换
    enum Mode { FAST, NORMAL, SLOW };
    Mode mode = NORMAL;
};

void test_config(int argc, char** argv) {
    auto& cfg = Config::getInstance();

	// 1. 注册 logging 模块
	LoggingConfig logging;
	cfg.registerConfig<LoggingConfig>("logging",
	    [](CLI::App* app, LoggingConfig& cfg) {
	        app->add_option("--log_level", cfg.level, "Log level (DEBUG, INFO, WARN, ERROR)");
	        app->add_option("--log_file", cfg.file, "Log file path");
	    });

	// 2. 注册 server 模块
	ServerConfig server;
	cfg.registerConfig<ServerConfig>("server",
	    [](CLI::App* app, ServerConfig& cfg) {
	        // 为枚举添加转换器
	        std::map<std::string, ServerConfig::Mode> mode_map = {
	            {"fast", ServerConfig::FAST},
	            {"normal", ServerConfig::NORMAL},
	            {"slow", ServerConfig::SLOW}
	        };
	        app->add_option("--mode", cfg.mode)
	            ->transform(CLI::CheckedTransformer(mode_map, CLI::ignore_case))
	            ->description("Select mode (fast/normal/slow)");
	        app->add_option("--port", cfg.port, "Server port")
	            ->check(CLI::Range(1024, 65535));
	        app->add_flag("--ssl", cfg.enable_ssl, "Enable SSL");
	    }, server);

	// 3. 解析命令行（自动处理 --config）
	cfg.parse(argc, argv);
	
    // 4. 通过 get 动态获取
    try {
        auto logging = cfg.get<LoggingConfig>("logging");
        std::cout << "Logging: level=" << logging.level << ", file=" << logging.file << std::endl;
        auto server = cfg.get<ServerConfig>("server");
		std::cout << "Server: port=" << server.port << ", ssl=" << server.enable_ssl
		          << ", mode=" << static_cast<int>(server.mode) << std::endl;
		          
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

//运行指令: ./a.out --config app.toml 
int main(int argc, char** argv) {
	test_config(argc, argv);
	return 0;
}
