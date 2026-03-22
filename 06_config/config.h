/**
 * @file config.h
 * @brief 配置管理器（单例），支持模块化配置、CLI11 命令行解析与 TOML 配置文件
 * 使用方式：
 * 1. 定义配置结构体（如 LoggingConfig）。
 * 2. 注册模块，提供 lambda 向子命令添加选项。
 * 3. 调用 parse() 解析命令行（自动加载 --config 指定的配置文件）。
 * 4. 通过 get<ConfigType>("module_name") 获取填充后的配置对象。
 *
 * @author yuguohua<ghy_hust@qq.com>
 * @date 2026.3.22
 * @copyright Copyright (c) 2026
 *
 * @version 1.0
 * @par Revision History:
 * - V1.0 2026.3.22 yuguohua<ghy_hust@qq.com>: Initial version
 */

#pragma once

#include <string>
#include <unordered_map>
#include <functional>
#include <memory>
#include <stdexcept>
#include <typeindex>
#include <CLI/CLI.hpp>

/**
 * @brief 配置管理器（单例）
 */
class Config {
public:
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
                        std::function<void(CLI::App*, T&)> config_func,
                        const T& default_value) {
        auto obj = std::make_shared<T>(default_value);
        return registerConfigImpl(name, config_func, obj);
    }

    // 注册模块（使用 T 的默认构造函数）
    template<typename T>
    void registerConfig(const std::string& name,
                        std::function<void(CLI::App*, T&)> config_func) {
        auto obj = std::make_shared<T>();
        return registerConfigImpl(name, config_func, obj);
    }

    // 解析命令行参数（自动加载 --config 文件）
    void parse(int argc, char** argv) {
        app_.parse(argc, argv);
    }

    // 获取配置对象副本
    template<typename T>
    T get(const std::string& name) const {
        auto it = config_objs_.find(name);
        if (it == config_objs_.end()) {
            throw std::runtime_error("Config module not found: " + name);
        }

        // 类型安全检查
        const auto& entry = it->second;
        if (entry.type != std::type_index(typeid(T))) {
            throw std::runtime_error("Type mismatch for config module: " + name +
                                     " (requested " + typeid(T).name() +
                                     ", stored " + entry.type.name() + ")");
        }

        return *static_cast<T*>(entry.obj.get());
    }

private:
    Config() : app_("Configuration Manager") {
        app_.set_config("--config", "", "Load configuration file", false);
        app_.allow_config_extras(CLI::config_extras_mode::ignore);
    }

    ~Config() = default;

    // 获取或创建子命令（对应 TOML 中的 [section]）
    CLI::App* getSubcommand(const std::string& name) {
        auto it = subcommands_.find(name);
        if (it != subcommands_.end()) {
        	return it->second;
        }
        
        auto sub = app_.add_subcommand(name, "Configuration for " + name);
        subcommands_[name] = sub;
        return sub;
    }

	template<typename T>
	void registerConfigImpl(const std::string& name,
                        std::function<void(CLI::App*, T&)> config_func,
                        std::shared_ptr<T> config_obj) {
		// 重复注册检查
        if (config_objs_.find(name) != config_objs_.end()) {
            throw std::runtime_error("Config module already registered: " + name);
        }

        CLI::App* sub = getSubcommand(name);
        config_func(sub, *config_obj);
        config_objs_.emplace(name, ConfigEntry{config_obj, std::type_index(typeid(T))});                        
    }
    
                        
private:
    struct ConfigEntry {
        std::shared_ptr<void> obj;
        std::type_index type;
        
        ConfigEntry(std::shared_ptr<void> o, std::type_index t) : obj(o), type(t) {}
    };
    
    CLI::App app_;
    std::unordered_map<std::string, CLI::App*> subcommands_;          // 子命令名 -> CLI 子命令对象
    std::unordered_map<std::string, ConfigEntry> config_objs_;        // 子命令名 -> 配置对象+类型
};
