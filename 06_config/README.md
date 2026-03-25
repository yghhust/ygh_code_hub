# 1. 概述

Config 是一个基于 CLI11 的单例配置管理器，旨在为 C++ 项目提供模块化、类型安全、支持配置文件的配置管理方案。它将命令行选项、配置文件（TOML/INI）与程序内部配置结构体解耦，允许用户按模块（子命令）组织配置，并在解析后通过统一的接口获取配置值。

该管理器核心特性包括：
* 模块化配置：每个配置模块对应一个子命令，自然映射到配置文件的节段（如 [logging]）。
* 类型安全：内部使用 std::shared_ptr<void> 存储任意类型，并通过 std::type_index 保证类型安全。
* 生命周期管理：配置对象由管理器持有，用户无需关心生命周期。
* CLI11 集成：无缝使用 CLI11 的选项定义、验证器、变换器等功能。
* 配置文件支持：通过 --config 或 -c 自动加载 TOML 配置文件（CLI11 内置）
* 便捷宏：提供宏简化注册、订阅、获取操作

# 2. 设计目标

* 易用性：用户只需定义配置结构体、注册模块、在 lambda 中添加选项，解析后即可获取结果。
* 扩展性：可轻松添加新的配置模块，或为现有模块添加新选项。
* 安全：防止重复注册、类型不匹配等误用。
* 与 CLI11 高度集成：不重复造轮子，直接利用 CLI11 的强大解析能力。
* 易用性：提供简洁的 API 和宏，减少样板代码

# 3. 核心组件
## 3.1 SubCmdEntry 类

每个已注册模块对应一个 SubCmdEntry 实例，负责管理模块的配置对象、CLI 子命令指针、类型信息、订阅者列表和脏标志。

## 3.2 Config

单例类，管理所有模块的 SubCmdEntry，提供注册、解析、获取和订阅接口。

# 4 工作流程
## 4.1 注册阶段

1. 用户调用 registerConfig<MyConfig>("module_name", parse_func)，传入解析函数（lambda 或普通函数）和可选默认值/回调。
2. Config 创建 CLI::App 子命令，并实例化配置对象（通过拷贝或默认构造）。
3. 创建 SubCmdEntry 存储对象指针、类型索引，并注册（如有）初始回调。
4. 调用用户解析函数，将 CLI 选项绑定到配置对象成员。
5. 遍历子命令的所有选项，为每个选项添加 each 回调，当选项被设置时调用 entry->set_changed() 标记脏。

## 4.2 解析阶段

1. 用户调用 parse(argc, argv)。
2. CLI11 解析命令行参数，同时自动加载 --config 指定的 TOML 文件（若存在）。
3. 对于每个被设置的选项，触发该选项的 each 回调，从而标记对应模块的 dirty 为 true。
4. 解析完成后，遍历所有 SubCmdEntry，调用 notify_if_changed()。
5. 若模块的 dirty 为 true，则遍历其 subscribers，逐个调用回调，传入配置对象（const T&）。
6. 清除 dirty 标志。

## 4.3 使用阶段

1. 获取配置：auto cfg = config.get<MyConfig>("module_name");，返回配置对象的副本，确保数据安全。
2. 订阅变更：config.subscribe<MyConfig>("module_name", callback);，回调将在下次 parse 且该模块配置变化时触发。

# 5 类型安全与错误处理

* 注册时：registerConfig 模板自动推导 T，内部通过 std::type_index 保存类型。
* 获取时：get<T> 会校验请求类型与存储类型是否一致，否则抛出 std::runtime_error。
* 订阅时：同样校验类型匹配，不匹配则抛出异常。
* 模块重复注册：抛出 runtime_error。
* 模块不存在：get 或 subscribe 时抛出 runtime_error。

# 6. 使用示例
## 6.1 定义配置结构体
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

6.2 注册模块并添加选项
```cpp
auto& cfg = Config::getInstance();

// 注册 logging 模块
cfg.registerConfig<LoggingConfig>("logging",
    [](CLI::App* app, LoggingConfig& cfg) {
        app->add_option("--level", cfg.level, "Log level");
        app->add_option("--file", cfg.file, "Log file");
    });  

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

6.3 解析命令行
```cpp
int main(int argc, char** argv) {
    auto& cfg = Config::getInstance();
    cfg.parse(argc, argv);  // 会自动处理 --config 选项

    auto logging = cfg.get<LoggingConfig>("logging");
    auto server = cfg.get<ServerConfig>("server");
    // 使用配置...
}
```

## 6.4 配置文件示例（config.toml）
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

# 7. 注意事项
* 重复注册：同一模块名不可重复注册，否则抛出异常。
* 类型匹配：get<T> 的模板参数必须与注册时使用的类型一致，否则抛出异常。
* 配置文件位置：默认配置文件为当前目录下的 config.toml，可通过命令行 --config 指定其他路径。
* 子命令名称：模块名会作为子命令名，在配置文件中对应的节段必须完全匹配（包括大小写）。
* 枚举处理：对于枚举类型，需要使用 CheckedTransformer 或自定义验证器，直接绑定会失败。
* 线程安全：Config 单例是线程安全的（C++11 静态局部变量），但 parse 应在单线程中调用（通常程序启动时）。若需要多线程访问 get，请确保在 parse 完成后才启动多线程。

