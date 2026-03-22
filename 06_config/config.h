/**
 * @file config.h
 * @brief 配置管理器（单例），支持模块化配置、CLI11 命令行解析与 TOML 配置文件
 * 使用方式：
 * 1. 定义配置结构体（如 LoggingConfig）。
 * 2. 注册模块，提供 lambda 向子命令添加选项。
 * 3. 调用 parse() 解析命令行（自动加载 --config 指定的配置文件）。
 * 4. 通过 get<ConfigType>("module_name") 获取填充后的配置对象。
 * 5. 通过 subscribe<ConfigType>("module_name", callback) 订阅配置变更。
 *
 * @author yuguohua<ghy_hust@qq.com>
 * @date 2026.3.22
 * @version 1.0
 */
#pragma once

#include <string>
#include <unordered_map>
#include <functional>
#include <memory>
#include <stdexcept>
#include <typeindex>
#include <vector>
#include <iostream>
#include <CLI/CLI.hpp>

// 配置数据更新回调（接收配置对象引用）
template<typename T>
using ONCHANGE_FUNC = std::function<int(const T&)>;

class SubCmdEntry {
public:
    SubCmdEntry(CLI::App* a, std::shared_ptr<void> o, std::type_index t)
        : app(a), obj(o), type(t), dirty(false) {}

    // 获取配置对象副本（类型安全）
    template<typename T>
    T get() const {
        if (type != std::type_index(typeid(T))) {
            throw std::runtime_error("Type mismatch: requested " +
                                     std::string(typeid(T).name()) +
                                     ", stored " + type.name());
        }
        return *static_cast<T*>(obj.get());
    }

    // 添加订阅者（回调接收 const T&）
    template<typename T>
    int subscribe(ONCHANGE_FUNC<T> callback) {
        if (type != std::type_index(typeid(T))) {
            return -1;
        }
        auto cb = [callback](void* ptr) -> int {
            return callback(*static_cast<const T*>(ptr));
        };
        subscribers.push_back(cb);
        return 0;
    }

    // 标记模块配置已变化
    void set_changed() {
        dirty = true;
    }

    // 触发变更通知（由 Config::parse 调用）
    void notify_if_changed() {
        if (!dirty) return;

        // 先调用模块级回调（如果存在）
        if (on_change_cb) {
            on_change_cb(obj.get());
        }

        // 再调用所有订阅者
        for (auto& cb : subscribers) {
            cb(obj.get());
        }

        dirty = false;
    }

private:
    CLI::App* app;                           // 子命令对象（用于获取选项）
    std::shared_ptr<void> obj;               // 配置对象
    std::type_index type;                    // 配置对象类型
    std::vector<std::function<int(void*)>> subscribers;  // 订阅者回调列表
    std::function<int(void*)> on_change_cb;  // 模块级回调（可选）
    bool dirty;                              // 脏标志
};

class Config {
public:
    // 配置命令解析回调
    template<typename T>
    using PARSE_FUNC = std::function<void(CLI::App*, T&)>;

    // 单例
    static Config& getInstance() {
        static Config instance;
        return instance;
    }

    // 禁止拷贝和移动
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    Config(Config&&) = delete;
    Config& operator=(Config&&) = delete;

public:
    // 注册模块（提供默认值）
    template<typename T>
    void registerConfig(const std::string& name,
                        PARSE_FUNC<T> parse_func,
                        const T& default_value,
                        ONCHANGE_FUNC<T> on_change = nullptr) {
        auto obj = std::make_shared<T>(default_value);
        registerConfigImpl(name, parse_func, obj, on_change);
    }

    // 注册模块（使用 T 的默认构造函数）
    template<typename T>
    void registerConfig(const std::string& name,
                        PARSE_FUNC<T> parse_func,
                        ONCHANGE_FUNC<T> on_change = nullptr) {
        auto obj = std::make_shared<T>();
        registerConfigImpl(name, parse_func, obj, on_change);
    }

    // 解析命令行参数（自动加载 --config 文件）
    void parse(int argc, char** argv) {
        // 1. 解析命令行和配置文件
        app_.parse(argc, argv);

        // 2. 通知所有脏模块（在解析过程中已通过选项回调设置 dirty 标志）
        for (auto& [name, entry] : subCmdEnts_) {
            entry->notify_if_changed();
        }
    }

    // 获取配置对象副本
    template<typename T>
    T get(const std::string& name) const {
        auto it = subCmdEnts_.find(name);
        if (it == subCmdEnts_.end()) {
            throw std::runtime_error("Config module not found: " + name);
        }
        return it->second->get<T>();
    }

    // 订阅指定模块的配置更新（回调函数接收 const T&）
    template<typename T>
    void subscribe(const std::string& name, ONCHANGE_FUNC<T> callback) {
        auto it = subCmdEnts_.find(name);
        if (it == subCmdEnts_.end()) {
            throw std::runtime_error("Config module not found: " + name);
        }
        if (it->second->subscribe<T>(callback) != 0) {
            throw std::runtime_error("Type mismatch for subscription: " + name);
        }
    }

private:
    Config() : app_("Configuration Manager") {
        app_.set_config("--config", "", "Load configuration file", false);
        app_.allow_config_extras(CLI::config_extras_mode::ignore);
    }

    ~Config() = default;

    // 内部实现：注册模块
    template<typename T>
    void registerConfigImpl(const std::string& name,
                            PARSE_FUNC<T> parse_func,
                            std::shared_ptr<T> config_obj,
                            ONCHANGE_FUNC<T> on_change) {
        // 重复注册检查
        if (subCmdEnts_.find(name) != subCmdEnts_.end()) {
            throw std::runtime_error("Config module already registered: " + name);
        }

        // 创建子命令
        CLI::App* sub = app_.add_subcommand(name, "Configuration for " + name);

        // 创建 SubCmdEntry 并存储
        auto entry = std::make_shared<SubCmdEntry>(sub, config_obj, std::type_index(typeid(T)));
        if (on_change) {
            if (entry->template subscribe<T>(on_change) != 0) {
                throw std::runtime_error("Failed to set module callback for: " + name);
            }
        }
        subCmdEnts_.emplace(name, entry);

        // 调用用户函数添加选项（绑定到 config_obj 的成员）
        parse_func(sub, *config_obj);

        // 为每个选项添加回调，当选项被设置时标记脏标志
        // 使用 opt->each 获取每个被设置的值的回调，注意 CLI11 的 each 是在解析过程中为每个值调用的
        for (auto* opt : sub->get_options()) {
            opt->each([entry](const std::string& value) {
                // 此处 value 是解析出的值，但我们只关心该选项被设置，不关心具体值
                entry->set_changed();
            });
        }
    }

private:
    CLI::App app_;
    std::unordered_map<std::string, std::shared_ptr<SubCmdEntry>> subCmdEnts_;
};
