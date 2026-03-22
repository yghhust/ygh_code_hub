# 1. 概述

Config 是一个基于 CLI11 的单例配置管理器，旨在为 C++ 项目提供模块化、类型安全、支持配置文件的配置管理方案。它将命令行选项、配置文件（TOML/INI）与程序内部配置结构体解耦，允许用户按模块（子命令）组织配置，并在解析后通过统一的接口获取配置值。

该管理器核心特性包括：
* 模块化配置：每个配置模块对应一个子命令，自然映射到配置文件的节段（如 [logging]）。
* 类型安全：内部使用 std::shared_ptr<void> 存储任意类型，并通过 std::type_index 保证类型安全。
* 生命周期管理：配置对象由管理器持有，用户无需关心生命周期。
* CLI11 集成：无缝使用 CLI11 的选项定义、验证器、变换器等功能。
* 配置文件支持：自动支持 --config 选项，加载 TOML/INI 文件，节段自动匹配子命令。

# 2. 设计目标

* 易用性：用户只需定义配置结构体、注册模块、在 lambda 中添加选项，解析后即可获取结果。
* 扩展性：可轻松添加新的配置模块，或为现有模块添加新选项。
* 安全：防止重复注册、类型不匹配等误用。
* 与 CLI11 高度集成：不重复造轮子，直接利用 CLI11 的强大解析能力。

# 3. 核心组件
## 3.1 Config 类（单例）

* 职责：管理所有配置模块的生命周期、子命令与选项的注册、命令行解析、配置值存储与获取。
* 访问：通过 Config::getInstance() 获取唯一实例。

## 3.2 ConfigEntry 结构

* 存储：std::shared_ptr<void> obj（实际配置对象的智能指针）和 std::type_index type（存储实际类型信息）。
* 用途：在 std::unordered_map 中统一管理不同类型的配置对象，同时支持类型检查。

## 3.3 内部数据结构

* subcommands_：std::unordered_map<std::string, CLI::App*>，存储子命令名到 CLI11 子命令对象的映射，用于复用已创建的子命令。
* config_objs_：std::unordered_map<std::string, ConfigEntry>，存储子命令名到配置对象及类型信息的映射。

## 3.4 关键方法
方法|	说明
:---|:---
getInstance()|单例访问点，返回全局唯一实例。
registerConfig<T>(name, config_func, default_value)|注册一个配置模块，使用给定的默认值初始化内部对象。
registerConfig<T>(name, config_func)|注册一个配置模块，使用 T 的默认构造函数初始化。
parse(argc, argv)|解析命令行，自动处理 --config 文件。
get<T>(name)|获取指定模块的配置对象副本，若类型不匹配或模块不存在则抛出异常。
getSubcommand(name)|获取或创建子命令对象（内部辅助）。
registerConfigImpl(...)|模板实现的具体注册逻辑，执行重复注册检查和最终存储。

# 4. 使用示例
## 4.1 定义配置结构体
```cpp
struct LoggingConfig {
    std::string level = "INFO";
    std::string file = "app.log";
};

struct ServerConfig {
    int port = 8080;
    bool enable_ssl = false;
    enum Mode { FAST, NORMAL, SLOW };
    Mode mode = NORMAL;
};
```

4.2 注册模块并添加选项
```cpp
auto& cfg = Config::getInstance();

// 注册 logging 模块
cfg.registerConfig<LoggingConfig>("logging",
    [](CLI::App* app, LoggingConfig& cfg) {
        app->add_option("--level", cfg.level, "Log level");
        app->add_option("--file", cfg.file, "Log file");
    }, LoggingConfig{});  // 提供默认值

// 注册 server 模块，使用枚举转换
cfg.registerConfig<ServerConfig>("server",
    [](CLI::App* app, ServerConfig& cfg) {
        std::map<std::string, ServerConfig::Mode> mode_map = {
            {"fast", ServerConfig::FAST},
            {"normal", ServerConfig::NORMAL},
            {"slow", ServerConfig::SLOW}
        };
        app->add_option("--mode", cfg.mode)
           ->transform(CLI::CheckedTransformer(mode_map, CLI::ignore_case));
        app->add_option("--port", cfg.port)->check(CLI::Range(1024, 65535));
        app->add_flag("--ssl", cfg.enable_ssl);
    });
```

4.3 解析命令行
```cpp
int main(int argc, char** argv) {
    auto& cfg = Config::getInstance();
    cfg.parse(argc, argv);  // 会自动处理 --config 选项

    auto logging = cfg.get<LoggingConfig>("logging");
    auto server = cfg.get<ServerConfig>("server");
    // 使用配置...
}
```

## 4.4 配置文件示例（config.toml）
```toml
[logging]
level = "DEBUG"
file = "debug.log"

[server]
port = 9090
ssl = true
mode = "fast"
```

运行命令：./myapp --config config.toml

# 5. 注意事项
* 重复注册：同一模块名不可重复注册，否则抛出异常。
* 类型匹配：get<T> 的模板参数必须与注册时使用的类型一致，否则抛出异常。
* 配置文件位置：默认配置文件为当前目录下的 config.toml，可通过命令行 --config 指定其他路径。
* 子命令名称：模块名会作为子命令名，在配置文件中对应的节段必须完全匹配（包括大小写）。
* 枚举处理：对于枚举类型，需要使用 CheckedTransformer 或自定义验证器，直接绑定会失败。
* 线程安全：Config 单例是线程安全的（C++11 静态局部变量），但 parse 应在单线程中调用（通常程序启动时）。若需要多线程访问 get，请确保在 parse 完成后才启动多线程。

