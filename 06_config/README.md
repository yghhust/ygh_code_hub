# 1. 概述

Config 是一个基于 CLI11 的单例配置管理器，旨在为 C++ 项目提供模块化、类型安全、支持 TOML 配置文件的配置管理方案。它将命令行选项、配置文件（TOML/INI）与程序内部配置结构体解耦，允许用户按模块（子命令）组织配置，并在配置变更时通过观察者模式自动通知订阅者。

该管理器核心特性包括：
* 模块化配置：每个配置模块对应一个子命令，自然映射到配置文件的节段（如 [logging]）。
* 类型安全：内部使用 std::shared_ptr<void> 存储任意类型，并通过 std::type_index 保证类型安全。
* 自动化变更检测：利用 CLI11 的选项回调（opt->each）标记脏标志，无需手动比较。
* 事件通知机制：支持模块级回调和多个订阅者，当配置变化时自动触发回调，传递最新配置副本。
* 生命周期管理：配置对象由管理器持有，用户无需关心生命周期。
* CLI11 集成：无缝使用 CLI11 的选项定义、验证器、变换器及 TOML 配置文件支持等功能。
* 配置文件支持：自动支持 --config 选项，加载 TOML/INI 文件，节段自动匹配子命令。

# 2. 设计目标

* 易用性：用户只需定义配置结构体、注册模块、在 lambda 中添加选项，解析后即可获取结果。
* 扩展性：可轻松添加新的配置模块，或为现有模块添加新选项。
* 安全：防止重复注册、类型不匹配等误用。
* 与 CLI11 高度集成：不重复造轮子，直接利用 CLI11 的强大解析能力。

# 3. 核心组件

## 3.1 Config 类（单例）

* 职责：管理所有配置模块的生命周期、子命令与选项的注册、命令行解析、配置值存储与获取、变更通知的调度。
* 访问：通过 Config::getInstance() 获取唯一实例。

## 3.2 SubCmdEntry 类

* 职责：封装单个配置模块的所有信息，包括：
    - app：指向 CLI11 子命令对象（用于选项遍历）。
    - obj：配置对象（std::shared_ptr<void>，类型擦除）。
    - type：配置对象的实际类型信息（std::type_index）。
    - subscribers：订阅者回调列表（类型擦除为 std::function<int(void*)>）。
    - dirty：脏标志，标记该模块是否有选项变化。

* 主要方法：
    - get<T>()：返回配置对象的副本，并检查类型一致性。
    - subscribe<T>(ONCHANGE_FUNC<T>)：添加订阅者，回调接收 const T&。
    - set_changed()：标记脏标志（由选项回调触发）。
    - notify_if_changed()：如果脏标志为真，依次调用模块回调、所有订阅者，然后清除脏标志。

## 3.3 类型别名

* PARSE_FUNC<T>：用户提供的解析函数类型，接收 CLI::App* 和 T&，用于向子命令添加选项。
* ONCHANGE_FUNC<T>：回调函数类型，接收 const T&，返回 int（可用于表示处理结果）。

# 4 关键流程

## 4.1 模块注册

1. 用户调用 registerConfig<T>(name, parse_func, [default_value], [on_change])。
2. Config 创建 std::shared_ptr<T> 对象（使用默认值或默认构造）。
3. 创建 SubCmdEntry 对象，存储配置对象、类型信息。
4. 如果提供了模块级回调 on_change，则调用 entry->template subscribe<T>(on_change) 将其作为订阅者加入（它优先于普通订阅者）。
5. 创建 CLI11 子命令（通过 app_.add_subcommand）。
6. 调用用户提供的 parse_func，让用户添加选项（绑定到配置对象的成员）。
7. 遍历子命令的所有选项，为每个选项设置 opt->each 回调。该回调在选项被设置时（无论是命令行还是配置文件）调用 entry->set_changed()，标记脏标志。
8. 将 SubCmdEntry 存储到 subCmdEnts_ 映射中。

## 4.2 命令行解析与配置加载

1. 用户调用 parse(argc, argv)。
2. app_.parse(argc, argv) 触发 CLI11 解析，自动处理 --config 指定的 TOML 文件。
3. 解析过程中，每个被设置的选项会触发其 opt->each 回调，将对应模块的脏标志设为 true。
4. 解析完成后，Config 遍历所有 SubCmdEntry，调用 notify_if_changed()：
5. 如果脏标志为 false，跳过。
6. 如果存在模块级回调，则调用它（传递配置对象的 void*，内部转换为 const T&）。
7. 依次调用所有订阅者回调。
8. 清除脏标志。

## 4.3 订阅机制

1. 用户通过 subscribe<T>(name, callback) 添加订阅者。
2. Config 查找对应模块的 SubCmdEntry，调用其 subscribe<T> 方法，将回调转换为类型擦除版本并加入订阅者列表。
3. 回调类型为 ONCHANGE_FUNC<T>，接收 const T&，确保只读访问。

## 4.4 配置值获取

1. 用户通过 get<T>(name) 获取配置对象副本。
2. Config 查找对应模块，调用 SubCmdEntry::get<T>()，返回 *static_cast<T*>(obj.get()) 的副本，并进行类型检查。

# 5. 使用示例
## 5.1 定义配置结构体
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

## 5.2 注册模块并添加选项
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

    // 添加额外订阅者
    cfg.subscribe<LoggingConfig>("logging", [](const LoggingConfig& c) {
        std::cout << "Subscriber: level=" << c.level << ", file=" << c.file << std::endl;
        return 0;
    });
```

## 5.3 解析命令行
```cpp
int main(int argc, char** argv) {
    auto& cfg = Config::getInstance();
    cfg.parse(argc, argv);  // 会自动处理 --config 选项

    auto logging = cfg.get<LoggingConfig>("logging");
    auto server = cfg.get<ServerConfig>("server");
    // 使用配置...
}
```

## 5.4 配置文件示例（config.toml）
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

# 6. 注意事项
* 重复注册：同一模块名不可重复注册，否则抛出异常。
* 类型匹配：get<T> 的模板参数必须与注册时使用的类型一致，否则抛出异常。
* 配置文件位置：默认配置文件为当前目录下的 config.toml，可通过命令行 --config 指定其他路径。
* 子命令名称：模块名会作为子命令名，在配置文件中对应的节段必须完全匹配（包括大小写）。
* 枚举处理：对于枚举类型，需要使用 CheckedTransformer 或自定义验证器，直接绑定会失败。
* 线程安全：Config 单例是线程安全的（C++11 静态局部变量），但 parse 应在单线程中调用（通常程序启动时）。若需要多线程访问 get，请确保在 parse 完成后才启动多线程。

# 7. 总结

Config 管理器提供了一个简洁、安全、可扩展的配置管理方案，将 CLI11 的强大能力与模块化设计、观察者模式相结合。它降低了配置管理的复杂性，使开发者能够专注于业务逻辑，同时保证配置变化的及时通知。该设计已在多个项目中使用，验证了其稳定性和可维护性。

