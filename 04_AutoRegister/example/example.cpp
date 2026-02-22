#include "auto_register.h"
#include <iostream>
#include <string>
#include <memory>

// ==================== 服务类定义 ====================

// 配置类
class Config {
public:
    std::string app_name = "DefaultApp";
    std::string db_host = "localhost";
    int db_port = 3306;
    bool debug_mode = false;
    
    void load(const std::string& filename) {
        std::cout << "[Config] Loading from " << filename << std::endl;
        // 模拟加载配置
    }
};

// 日志类
class Logger {
public:
    std::string level = "INFO";
    
    void setLevel(const std::string& lvl) {
        level = lvl;
        std::cout << "[Logger] Level set to: " << level << std::endl;
    }
    
    void log(const std::string& msg) {
        std::cout << "[" << level << "] " << msg << std::endl;
    }
};

// 数据库服务
class DatabaseService {
public:
    std::string connection_string;
    std::shared_ptr<Config> config;
    std::shared_ptr<Logger> logger;
    bool connected = false;
    
    void connect() {
        connection_string = "mysql://" + config->db_host + ":" + std::to_string(config->db_port);
        connected = true;
        logger->log("Connected to: " + connection_string);
    }
    
    void disconnect() {
        connected = false;
        logger->log("Disconnected");
    }
};

// API 服务
class ApiService {
public:
    std::string base_url;
    int port = 8080;
    std::shared_ptr<DatabaseService> database;
    std::shared_ptr<Logger> logger;
    
    void start() {
        base_url = "http://localhost:" + std::to_string(port);
        logger->log("API started on " + base_url);
    }
    
    void stop() {
        logger->log("API stopped");
    }
};

// ==================== 外部依赖（将在 lambda 中捕获） ====================
 #if 0
 AUTO_REGISTER_LAMBDA_CREATOR(DatabaseService,
        ([]() {
            auto db = std::make_shared<DatabaseService>();
            //db->config = config;
            //db->logger = logger;
            return db;
        })
    );
#endif
// 全局/外部依赖
std::shared_ptr<Config> g_config;
std::shared_ptr<Logger> g_logger;
bool g_enable_feature_x = true;

// ==================== 主函数演示 ====================

int main() {
    std::cout << "=== Lambda 定制注册演示 ===" << std::endl;
    
    // 1. 准备外部依赖
    std::cout << "\n--- 步骤1: 准备外部依赖 ---" << std::endl;
    g_config = std::make_shared<Config>();
    g_config->app_name = "MyApplication";
    g_config->db_host = "192.168.1.100";
    g_config->db_port = 5432;
    g_config->debug_mode = true;
    
    g_logger = std::make_shared<Logger>();
    g_logger->setLevel("DEBUG");
    
    std::cout << "配置已准备: " << g_config->app_name << std::endl;
    std::cout << "日志已准备: " << g_logger->level << std::endl;
    
    // 2. 使用 Lambda 定制注册服务
    std::cout << "\n--- 步骤2: Lambda 定制注册 ---" << std::endl;
    
    // 方式1：最简 Lambda 注册（捕获外部变量）
    std::cout << "\n[方式1] 最简 Lambda 注册:" << std::endl;
    AUTO_REGISTER_LAMBDA_CREATOR(DatabaseService, 
	    ([config = g_config, logger = g_logger]() {
	    auto db = std::make_shared<DatabaseService>();
	    db->config = config;
	    db->logger = logger;
	    return db;
	    })
    );
    
    #if 1
    // 方式2：带初始化的 Lambda 注册
    std::cout << "\n[方式2] 带初始化的 Lambda 注册:" << std::endl;
    AUTO_REGISTER_LAMBDA_CREATOR_WITH_INIT(ApiService,
        [logger = g_logger]() {
            auto api = std::make_shared<ApiService>();
            api->logger = logger;
            api->port = 3000;  // 覆盖默认值
            return api;
        },
        [](ApiService& api) {
            // 显式初始化（此时可以访问已注入的依赖）
            api.database = AutoRegister::instance().getInstance<DatabaseService>("DatabaseService");
            api.start();
        }
    );
    
    // 方式3：条件创建 Lambda 注册
    std::cout << "\n[方式3] 条件创建 Lambda 注册:" << std::endl;
    AUTO_REGISTER_LAMBDA_CREATOR(FeatureXService,
        [enabled = g_enable_feature_x]() -> std::shared_ptr<void> {
            if (!enabled) {
                std::cout << "[FeatureX] Feature disabled, returning null" << std::endl;
                return nullptr;
            }
            std::cout << "[FeatureX] Creating feature service" << std::endl;
            // 返回实际的服务实例
            return std::make_shared<int>(42);  // 示例
        }
    );
    
    // 方式4：复杂创建逻辑的 Lambda 注册
    std::cout << "\n[方式4] 复杂创建逻辑 Lambda 注册:" << std::endl;
    AUTO_REGISTER_LAMBDA_CREATOR(ComplexService,
        ([&config = g_config, &logger = g_logger]() {
            std::cout << "[ComplexService] Starting complex initialization..." << std::endl;
            
            // 创建多个子组件
            auto service = std::make_shared<DatabaseService>();
            service->config = config;
            service->logger = logger;
            
            // 执行验证
            if (config->db_port <= 0 || config->db_port > 65535) {
                throw std::runtime_error("Invalid database port");
            }
            
            std::cout << "[ComplexService] Complex initialization complete" << std::endl;
            return service;
        })
    );
    
    // 方式5：命名实例 Lambda 注册
    std::cout << "\n[方式5] 命名实例 Lambda 注册:" << std::endl;
    AUTO_REGISTER_LAMBDA_NAMED_CREATOR(Config, primary,
        [config = g_config]() {
            auto cfg = std::make_shared<Config>();
            *cfg = *config;  // 复制配置
            cfg->app_name = "PrimaryConfig";
            return cfg;
        }
    );
    
    AUTO_REGISTER_LAMBDA_NAMED_CREATOR(Config, secondary,
        []() {
            auto cfg = std::make_shared<Config>();
            cfg->app_name = "SecondaryConfig";
            cfg->db_host = "backup-server";
            cfg->db_port = 3306;
            return cfg;
        }
    );
    #endif
    // 打印注册状态
    std::cout << "\n--- 注册状态 ---" << std::endl;
    AutoRegister::instance().printCreators();
    AutoRegister::instance().printInitQueues();
    
    // 3. 执行初始化
    std::cout << "\n--- 步骤3: 执行初始化 ---" << std::endl;
    AutoRegister::instance().executeAllInits();
    
    // 4. 获取和使用实例
    std::cout << "\n--- 步骤4: 获取和使用实例 ---" << std::endl;
    
    // 获取数据库服务
    auto& reg = AutoRegister::instance();
    auto db_service = reg.getInstance<DatabaseService>("DatabaseService");
    if (db_service) {
        std::cout << "\n数据库服务:" << std::endl;
        std::cout << "  - 连接字符串: " << db_service->connection_string << std::endl;
        std::cout << "  - 已连接: " << (db_service->connected ? "是" : "否") << std::endl;
        std::cout << "  - 配置来源: " << db_service->config->app_name << std::endl;
    }
    
    // 获取 API 服务
    auto api_service = reg.getInstance<ApiService>("ApiService");
    if (api_service) {
        std::cout << "\nAPI 服务:" << std::endl;
        std::cout << "  - 基础 URL: " << api_service->base_url << std::endl;
        std::cout << "  - 端口: " << api_service->port << std::endl;
        std::cout << "  - 数据库依赖: " << (api_service->database ? "已注入" : "未注入") << std::endl;
    }
    
    // 获取命名配置实例
    auto primary_config = reg.getInstanceByName<Config>("primary", "Config");
    auto secondary_config = reg.getInstanceByName<Config>("secondary", "Config");
    
    if (primary_config) {
        std::cout << "\n主配置:" << std::endl;
        std::cout << "  - 应用名: " << primary_config->app_name << std::endl;
        std::cout << "  - 数据库主机: " << primary_config->db_host << std::endl;
    }
    
    if (secondary_config) {
        std::cout << "\n次配置:" << std::endl;
        std::cout << "  - 应用名: " << secondary_config->app_name << std::endl;
        std::cout << "  - 数据库主机: " << secondary_config->db_host << std::endl;
    }
    
    // 获取条件服务（可能为空）
    //auto feature_x = reg.getInstance<FeatureXService>();
    //std::cout << "\n功能X服务: " << (feature_x ? "已创建" : "未创建（已禁用）") << std::endl;
    
    // 打印最终状态
    std::cout << "\n--- 最终状态 ---" << std::endl;
    reg.printInstances();
    
    std::cout << "\n=== 演示完成 ===" << std::endl;
    
    return 0;
}
