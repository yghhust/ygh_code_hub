/**
 * @file auto_register.h
 * @brief 自动注册与惰性实例化框架
 *
 * @author yuguohua<ghy_hust@qq.com>
 * @date 2026.2.25
 * @copyright Copyright (c) 2026
 *
 * @version 1.5
 * @par Revision History:
 * - V1.0 2026.2.3  yuguohua<ghy_hust@qq.com>: Initial version
 * - V1.1 2026.2.6  yuguohua<ghy_hust@qq.com>: 新增优先级初始化功能
 * - V1.2 2026.2.7  yuguohua<ghy_hust@qq.com>: 新增多实例支持功能
 * - V1.3 2026.2.8  yuguohua<ghy_hust@qq.com>: 优化锁设计与执行流程（解决死锁问题）
 * - V1.4 2026.2.22 yuguohua<ghy_hust@qq.com>: 支持带参构造
 * - V1.5 2026.2.25  yuguohua<ghy_hust@qq.com>: 核心优化，使用 typeid(T).name() 自动获取类名，移除所有手动类名参数。
 *
 * @section overview 概述 * 
 * 本文件实现了一个通用的类/对象自动注册系统，支持：
 * - 自动按类型名注册与查找（无需手动传递类名）
 * - 支持无参构造及自定义 Lambda 创建器
 * - 支持带初始化的注册（初始化函数可在首次获取实例时或执行阶段调用）
 * - 支持优先级控制初始化顺序
 * - 支持宏方式在全局作用域自动完成注册
 * - 线程安全（使用 std::mutex 保护注册表与实例缓存）
 *
 * 主要用途：
 * 1. 在大型项目中集中管理类的创建与初始化，避免手动 new/delete
 * 2. 实现插件化架构，通过注册表动态创建对象
 * 3. 控制初始化顺序，解决跨模块依赖
 *
 * 使用方式：
 * - 在类定义文件中使用宏 AUTO_REGISTER_CLASS、AUTO_REGISTER_CLASS_WITH_INITFUNC 等完成自动注册
 * - 在需要实例时调用 AutoRegister::instance().getInstance<T>()
 * - 在程序启动阶段调用 executeAllInits() 或 executePriorInits() 执行初始化
 *
 * 注意事项：
 * - 注册宏必须在全局或命名空间作用域使用，不能在函数内（因涉及静态变量初始化）
 * - 对于带初始化的注册，初始化函数签名需匹配（如 std::function<void(T&)>）
 * - 本实现使用 shared_ptr<void> 作为统一存储，确保不同类型可共存于同一注册表
 * - 使用 typeid(T).name() 作为键，其输出是编译器相关的，但能保证唯一性
 *
 */

#pragma once
#include <unordered_map>
#include <memory>
#include <string>
#include <typeinfo>
#include <vector>
#include <functional>
#include <mutex>
#include <iostream>
#include <algorithm>
#include <utility>
#include <sstream>
#include <cstdint>

class AutoRegister {
private:
    AutoRegister() = default;
    AutoRegister(const AutoRegister&) = delete;
    AutoRegister& operator=(const AutoRegister&) = delete;
    AutoRegister(AutoRegister&&) = delete;
    AutoRegister& operator=(AutoRegister&&) = delete;

public:
    static AutoRegister& instance() {
        static AutoRegister instance;
        return instance;
    }

private:
    mutable std::mutex mutex_;

    // 统一的注册条目结构
    struct RegistrationEntry {
        std::function<std::shared_ptr<void>()> creator;
        std::function<void()> initializer;
        int priority = 5;
    };

    // 核心存储
    std::unordered_map<std::string, RegistrationEntry> registry_;
    std::unordered_map<std::string, std::shared_ptr<void>> instances_;

    // 核心辅助函数：生成唯一的注册键
    template<typename T>
    static std::string makeKey() {
        return std::string(typeid(T).name());
    }

    template<typename T>
    static std::string makeKey(const std::string& instance_name) {
        return std::string(typeid(T).name()) + "#" + instance_name;
    }
   
    // 核心实现：所有注册最终走这里
    template<typename T, typename CreatorFunc, typename InitFunc = std::nullptr_t>
    void registerEntryImpl(const std::string& key,
                           CreatorFunc&& creator,
                           InitFunc&& init = nullptr,
                           int priority = 5) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (registry_.count(key)) {
            std::cerr << "[AutoRegister WARNING] Overwriting registration for '" << key << "'\n";
        }
        priority = std::clamp(priority, 0, 10);

        // 包装 creator 使其返回 shared_ptr<void>
        auto voidCreator = [creator = std::forward<CreatorFunc>(creator)]() -> std::shared_ptr<void> {
            return creator();
        };

        // 包装 initializer
        auto initWrapper = [this, key, initFunc = std::forward<InitFunc>(init)]() {
            auto inst = lazyCreateInstanceImpl<T>(key);
            if constexpr (!std::is_same_v<std::decay_t<InitFunc>, std::nullptr_t>) {
               initFunc(*std::static_pointer_cast<T>(inst));
            }
        };
        
        registry_[key] = {std::move(voidCreator), std::move(initWrapper), priority};
        std::cout << "[AutoRegister INFO] Registered '" << key << "' with priority " << priority << "\n";
    }

public:
    /**
     * @brief 统一注册入口（无初始化函数版本）
     * @tparam T 注册的类型
     * @tparam CreatorFunc 创建函数类型，签名 std::shared_ptr<T>()
     * @param creator 创建函数，返回 T 的 shared_ptr
     * @param priority 初始化优先级 [0-10]，默认 5
     * @note 类名通过 typeid(T).name() 自动获取，无需手动指定。
     */
    template<typename T, typename CreatorFunc>
    void registerEntry(CreatorFunc&& creator, int priority = 5) {
        registerEntryImpl<T>(makeKey<T>(), std::forward<CreatorFunc>(creator), nullptr, priority);
    }

    /**
     * @brief 统一注册入口（带初始化函数版本）
     * @tparam T 注册的类型
     * @tparam CreatorFunc 创建函数类型，签名 std::shared_ptr<T>()
     * @tparam InitFunc 初始化函数类型，签名 void(T&)
     * @param creator 创建函数，返回 T 的 shared_ptr
     * @param init 初始化函数，接收 T& 引用
     * @param priority 初始化优先级 [0-10]，默认 5
     * @note 类名通过 typeid(T).name() 自动获取，无需手动指定。
     */
    template<typename T, typename CreatorFunc, typename InitFunc>
    void registerEntryWithInit(CreatorFunc&& creator,
                               InitFunc&& init,
                               int priority = 5) {
        registerEntryImpl<T>(makeKey<T>(), 
                            std::forward<CreatorFunc>(creator), 
                            std::forward<InitFunc>(init), 
                            priority);
    }

    // ==================== 对外提供的便捷注册函数 ====================

    /**
     * @brief 基础 Lambda 创建器 (自动推导类名)
     */
    template<typename T>
    void registerCreator(std::function<std::shared_ptr<T>()> creator_lambda,
                        int priority = 5) {
        registerEntry<T>(std::move(creator_lambda), priority);
    }

    /**
     * @brief 带初始化的 Lambda 创建器 (自动推导类名)
     */
    template<typename T>
    void registerCreatorWithInit(std::function<std::shared_ptr<T>()> creator_lambda,
                                 std::function<void(T&)> init_lambda,
                                 int priority = 5) {
        registerEntryWithInit<T>(std::move(creator_lambda), std::move(init_lambda), priority);
    }

    /**
     * @brief 命名实例注册 (自动推导类名)
     * @param instance_name 实例名称，用于区分同类型的不同实例
     * @note 注册键为 typeid(T).name() + "#" + instance_name
     */
    template<typename T>
    void registerNamedCreator(const std::string& instance_name,
                             std::function<std::shared_ptr<T>()> creator_lambda,
                             int priority = 5) {
        registerEntryImpl<T>(makeKey<T>(instance_name), std::move(creator_lambda), nullptr, priority);
    }

    /**
     * @brief 带初始化的命名实例注册 (自动推导类名)
     * @param instance_name 实例名称，用于区分同类型的不同实例
     * @note 注册键为 typeid(T).name() + "#" + instance_name
     */
    template<typename T>
    void registerNamedCreatorWithInit(const std::string& instance_name,
                                      std::function<std::shared_ptr<T>()> creator_lambda,
                                      std::function<void(T&)> init_lambda,
                                      int priority = 5) {
        registerEntryImpl<T>(makeKey<T>(instance_name), std::move(creator_lambda), std::move(init_lambda), priority);
    }

    /**
     * @brief 类自动注册 (无参构造, 自动推导类名)
     */
    template<typename T>
    void registerClass(int priority = 5) {
        registerEntry<T>([]() { return std::make_shared<T>(); }, priority);
    }

    /**
     * @brief 类自动注册 (带初始化函数, 自动推导类名)
     */
    template<typename T>
    void registerClassWithInit(std::function<void(T&)> init_func,
                               int priority = 5) {
        registerEntryWithInit<T>([]() { return std::make_shared<T>(); }, std::move(init_func), priority);
    }

    /**
     * @brief 命名类实例自动注册 (自动推导类名)
     */
    template<typename T>
    void registerNamedInstance(const std::string& instance_name,
                              int priority = 5) {
        registerEntryImpl<T>(makeKey<T>(instance_name), []() { return std::make_shared<T>(); }, nullptr, priority);
    }

    /**
     * @brief 命名类实例自动注册 (带初始化, 自动推导类名)
     */
    template<typename T>
    void registerNamedInstanceWithInit(const std::string& instance_name,
                                       std::function<void(T&)> init_func,
                                       int priority = 5) {
        registerEntryImpl<T>(makeKey<T>(instance_name), []() { return std::make_shared<T>(); }, std::move(init_func), priority);
    }

    // ==================== 初始化执行 ====================

    void executeAllInits() {
        executePriorInits(10);
    }

    void executePriorInits(int max_priority = 10) {
        max_priority = std::clamp(max_priority, 0, 10);
        auto inits = collectInitsUpToPriority(max_priority);
        executeAllCollectedInits(std::move(inits),
            "[AutoRegister::Init] Starting execution of initializers with priority 0-" + std::to_string(max_priority) + "...");
        {
            std::lock_guard<std::mutex> lock(mutex_);
            std::cout << "[AutoRegister::Init] Priority 0-" << max_priority << " initializers executed. Total instances: "
                      << instances_.size() << std::endl;
        }
    }

    // ==================== 实例获取 ====================

    /**
     * @brief 获取单例实例 (通过类型 T 自动查找)
     */
    template<typename T>
    std::shared_ptr<T> getInstance() {
        std::lock_guard<std::mutex> lock(mutex_);
        std::string key = makeKey<T>();
        auto it = instances_.find(key);
        if (it != instances_.end()) {
            return std::static_pointer_cast<T>(it->second);
        }
        return lazyCreateInstanceImpl<T>(key);
    }

    /**
     * @brief 获取命名实例 (通过类型 T 和实例名自动查找)
     */
    template<typename T>
    std::shared_ptr<T> getInstanceByName(const std::string& instance_name) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::string full_name = makeKey<T>(instance_name);
        auto it = instances_.find(full_name);
        if (it != instances_.end()) {
            return std::static_pointer_cast<T>(it->second);
        }
        return lazyCreateInstanceImpl<T>(full_name);
    }

    template<typename T>
    bool hasInstance() {
        std::lock_guard<std::mutex> lock(mutex_);
        return instances_.find(makeKey<T>()) != instances_.end();
    }

    template<typename T>
    bool hasNamedInstance(const std::string& instance_name) {
        std::lock_guard<std::mutex> lock(mutex_);
        return instances_.find(makeKey<T>(instance_name)) != instances_.end();
    }

    template<typename T>
    std::shared_ptr<T> createTempInstance() {
        return std::make_shared<T>();
    }

    // ==================== 调试与工具 ====================

    void printInstances() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::cout << "\n[AutoRegister::Debug] Registered instances (" << instances_.size() << "):" << std::endl;
        if (instances_.empty()) {
            std::cout << "  No instances registered." << std::endl;
            return;
        }
        for (const auto& pair : instances_) {
            std::cout << "  - Key: " << pair.first << ", Address: " << pair.second.get() << std::endl;
        }
        std::cout << std::endl;
    }

    void printRegistry() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::cout << "\n[AutoRegister::Debug] Registry entries (" << registry_.size() << "):" << std::endl;
        for (const auto& pair : registry_) {
            std::cout << "  - Key: " << pair.first << ", Priority: " << pair.second.priority << std::endl;
        }
        std::cout << std::endl;
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        instances_.clear();
        registry_.clear();
    }

    size_t getInstanceCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return instances_.size();
    }

    size_t getRegisteredCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return registry_.size();
    }

private:

    // ==================== 内部实现 ====================
    // 核心惰性创建实现（带类型参数）
    template<typename T>
    std::shared_ptr<T> lazyCreateInstanceImpl(const std::string& key) {
        auto it = instances_.find(key);
        if (it != instances_.end()) {
            return std::static_pointer_cast<T>(it->second);
        }
        auto regIt = registry_.find(key);
        if (regIt == registry_.end()) {
            std::cerr << "[AutoRegister ERROR] No registration for '" << key << "'\n";
            return nullptr;
        }
        try {
            auto inst = regIt->second.creator();
            if (inst) {
                instances_[key] = inst;
            }
            return std::static_pointer_cast<T>(inst);
        } catch (const std::exception& e) {
            std::cerr << "[AutoRegister ERROR] Create failed for '" << key << "': " << e.what() << "\n";
            return nullptr;
        }
    }

    std::vector<std::function<void()>> collectInitsUpToPriority(int max_priority) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<std::pair<int, std::function<void()>>> sorted_inits;
        for (const auto& [key, entry] : registry_) {
            if (entry.initializer) {
                sorted_inits.emplace_back(entry.priority, entry.initializer);
            }
        }
        std::sort(sorted_inits.begin(), sorted_inits.end(),
                  [](const auto& a, const auto& b) { return a.first < b.first; });

        std::vector<std::function<void()>> result;
        for (const auto& [pri, func] : sorted_inits) {
            if (pri <= max_priority) {
                result.push_back(func);
            }
        }
        return result;
    }

    void executeAllCollectedInits(std::vector<std::function<void()>> inits, const std::string& description) {
        std::cout << description << " (" << inits.size() << " items)..." << std::endl;
        for (size_t i = 0; i < inits.size(); ++i) {
            try { inits[i](); }
            catch (const std::exception& e) {
                std::cerr << "[ERROR] AutoRegister Initializer " << i << " failed: " << e.what() << std::endl;
            }
            catch (...) {
                std::cerr << "[ERROR] AutoRegister Initializer " << i << " failed with unknown exception." << std::endl;
            }
        }
    }
};

// ==================== 统一宏实现 ====================

#define _AUTO_REGISTER_IMPL(UNIQUE_NAME, REGISTER_CALL) \
    namespace { \
        struct UNIQUE_NAME##_Helper { \
            UNIQUE_NAME##_Helper() { \
                REGISTER_CALL; \
            } \
        }; \
        static UNIQUE_NAME##_Helper UNIQUE_NAME##_instance; \
    }

// ==================== Class Init Macros ====================

#define AUTO_REGISTER_CLASS(CLASS_NAME) \
    AUTO_REGISTER_CLASS_PRIORITY(CLASS_NAME, 5)

#define AUTO_REGISTER_CLASS_PRIORITY(CLASS_NAME, PRIORITY) \
    _AUTO_REGISTER_IMPL( \
        CLASS_NAME##_AutoRegister, \
        ::AutoRegister::instance().registerClass<CLASS_NAME>(PRIORITY) \
    )

#define AUTO_REGISTER_CLASS_WITH_INITFUNC(CLASS_NAME, INIT_MEMBER_FUNC) \
    AUTO_REGISTER_CLASS_WITH_INITFUNC_PRIORITY(CLASS_NAME, INIT_MEMBER_FUNC, 5)

#define AUTO_REGISTER_CLASS_WITH_INITFUNC_PRIORITY(CLASS_NAME, INIT_MEMBER_FUNC, PRIORITY) \
    _AUTO_REGISTER_IMPL( \
        CLASS_NAME##_AutoRegister, \
        ::AutoRegister::instance().registerClassWithInit<CLASS_NAME>( \
            [](CLASS_NAME& obj) { obj.INIT_MEMBER_FUNC(); }, PRIORITY) \
    )

#define AUTO_REGISTER_CLASS_WITH_INIT(CLASS_NAME, INIT_LAMBDA) \
    AUTO_REGISTER_CLASS_WITH_INIT_PRIORITY(CLASS_NAME, INIT_LAMBDA, 5)

#define AUTO_REGISTER_CLASS_WITH_INIT_PRIORITY(CLASS_NAME, INIT_LAMBDA, PRIORITY) \
    _AUTO_REGISTER_IMPL( \
        CLASS_NAME##_AutoRegister, \
        ::AutoRegister::instance().registerClassWithInit<CLASS_NAME>( \
            INIT_LAMBDA, PRIORITY) \
    )
    
// ==================== Class Instance Macros ====================

#define AUTO_REGISTER_CLASS_INSTANCE(CLASS_NAME, INSTANCE_NAME) \
    _AUTO_REGISTER_IMPL( \
        CLASS_NAME##_##INSTANCE_NAME##_AutoRegister, \
        ::AutoRegister::instance().registerNamedInstance<CLASS_NAME>(#INSTANCE_NAME, 5) \
    )

#define AUTO_REGISTER_CLASS_INSTANCE_WITH_INITFUNC(CLASS_NAME, INSTANCE_NAME, INIT_MEMBER_FUNC) \
    AUTO_REGISTER_CLASS_INSTANCE_WITH_INITFUNC_PRIORITY(CLASS_NAME, INSTANCE_NAME, INIT_MEMBER_FUNC, 5)

#define AUTO_REGISTER_CLASS_INSTANCE_WITH_INITFUNC_PRIORITY(CLASS_NAME, INSTANCE_NAME, INIT_MEMBER_FUNC, PRIORITY) \
    _AUTO_REGISTER_IMPL( \
        CLASS_NAME##_##INSTANCE_NAME##_AutoRegister, \
        ::AutoRegister::instance().registerNamedInstanceWithInit<CLASS_NAME>(#INSTANCE_NAME, \
            [](CLASS_NAME& obj) { obj.INIT_MEMBER_FUNC(); }, PRIORITY) \
    )

#define AUTO_REGISTER_CLASS_INSTANCE_WITH_INIT(CLASS_NAME, INSTANCE_NAME, INIT_LAMBDA) \
    AUTO_REGISTER_CLASS_INSTANCE_WITH_INIT_PRIORITY(CLASS_NAME, INSTANCE_NAME, INIT_LAMBDA, 5)

#define AUTO_REGISTER_CLASS_INSTANCE_WITH_INIT_PRIORITY(CLASS_NAME, INSTANCE_NAME, INIT_LAMBDA, PRIORITY) \
    _AUTO_REGISTER_IMPL( \
        CLASS_NAME##_##INSTANCE_NAME##_AutoRegister, \
        ::AutoRegister::instance().registerNamedInstanceWithInit<CLASS_NAME>(#INSTANCE_NAME, \
            INIT_LAMBDA, PRIORITY) \
    )


// ==================== Class Creator Macros ====================

// 基础类创建器（默认优先级 5）
#define AUTO_REGISTER_CLASS_CREATOR(CLASS_NAME, CREATOR_LAMBDA) \
    AUTO_REGISTER_CLASS_CREATOR_PRIORITY(CLASS_NAME, CREATOR_LAMBDA, 5)

// 基础类创建器（带优先级）
#define AUTO_REGISTER_CLASS_CREATOR_PRIORITY(CLASS_NAME, CREATOR_LAMBDA, PRIORITY) \
    _AUTO_REGISTER_IMPL( \
        CLASS_NAME##_ClassCreatorAutoRegister, \
        ::AutoRegister::instance().registerCreator<CLASS_NAME>(CREATOR_LAMBDA, PRIORITY) \
    )

// 带成员函数初始化的类创建器（默认优先级 5）
#define AUTO_REGISTER_CLASS_CREATOR_WITH_INITFUNC(CLASS_NAME, CREATOR_LAMBDA, INIT_MEMBER_FUNC) \
    AUTO_REGISTER_CLASS_CREATOR_WITH_INITFUNC_PRIORITY(CLASS_NAME, CREATOR_LAMBDA, INIT_MEMBER_FUNC, 5)

// 带成员函数初始化的类创建器（带优先级）
#define AUTO_REGISTER_CLASS_CREATOR_WITH_INITFUNC_PRIORITY(CLASS_NAME, CREATOR_LAMBDA, INIT_MEMBER_FUNC, PRIORITY) \
    _AUTO_REGISTER_IMPL( \
        CLASS_NAME##_ClassCreatorInitFuncAutoRegister, \
        ::AutoRegister::instance().registerCreatorWithInit<CLASS_NAME>(CREATOR_LAMBDA, \
            [](CLASS_NAME& obj){ obj.INIT_MEMBER_FUNC(); }, PRIORITY) \
    )

// 带 Lambda 初始化的类创建器（默认优先级 5）
#define AUTO_REGISTER_CLASS_CREATOR_WITH_INIT(CLASS_NAME, CREATOR_LAMBDA, INIT_LAMBDA) \
    AUTO_REGISTER_CLASS_CREATOR_WITH_INIT_PRIORITY(CLASS_NAME, CREATOR_LAMBDA, INIT_LAMBDA, 5)

// 带 Lambda 初始化的类创建器（带优先级）
#define AUTO_REGISTER_CLASS_CREATOR_WITH_INIT_PRIORITY(CLASS_NAME, CREATOR_LAMBDA, INIT_LAMBDA, PRIORITY) \
    _AUTO_REGISTER_IMPL( \
        CLASS_NAME##_ClassCreatorInitAutoRegister, \
        ::AutoRegister::instance().registerCreatorWithInit<CLASS_NAME>(CREATOR_LAMBDA, INIT_LAMBDA, PRIORITY) \
    )

// 命名实例的类创建器（默认优先级 5）
#define AUTO_REGISTER_CLASS_CREATOR_INSTANCE(CLASS_NAME, INSTANCE_NAME, CREATOR_LAMBDA) \
    AUTO_REGISTER_CLASS_CREATOR_INSTANCE_PRIORITY(CLASS_NAME, INSTANCE_NAME, CREATOR_LAMBDA, 5)

// 命名实例的类创建器（带优先级）
#define AUTO_REGISTER_CLASS_CREATOR_INSTANCE_PRIORITY(CLASS_NAME, INSTANCE_NAME, CREATOR_LAMBDA, PRIORITY) \
    _AUTO_REGISTER_IMPL( \
        CLASS_NAME##_##INSTANCE_NAME##_ClassCreatorAutoRegister, \
        ::AutoRegister::instance().registerNamedCreator<CLASS_NAME>(#INSTANCE_NAME, CREATOR_LAMBDA, PRIORITY) \
    )

// 带成员函数初始化的命名实例的类创建器（默认优先级 5）
#define AUTO_REGISTER_CLASS_CREATOR_INSTANCE_WITH_INITFUNC(CLASS_NAME, INSTANCE_NAME, CREATOR_LAMBDA, INIT_MEMBER_FUNC) \
    AUTO_REGISTER_CLASS_CREATOR_INSTANCE_WITH_INITFUNC_PRIORITY(CLASS_NAME, INSTANCE_NAME, CREATOR_LAMBDA, INIT_MEMBER_FUNC, 5)

// 带成员函数初始化的命名实例的类创建器（带优先级）
#define AUTO_REGISTER_CLASS_CREATOR_INSTANCE_WITH_INITFUNC_PRIORITY(CLASS_NAME, INSTANCE_NAME, CREATOR_LAMBDA, INIT_MEMBER_FUNC, PRIORITY) \
    _AUTO_REGISTER_IMPL( \
        CLASS_NAME##_##INSTANCE_NAME##_ClassCreatorInitFuncAutoRegister, \
        ::AutoRegister::instance().registerNamedCreatorWithInit<CLASS_NAME>(#INSTANCE_NAME, CREATOR_LAMBDA, \
            [](CLASS_NAME& obj){ obj.INIT_MEMBER_FUNC(); }, PRIORITY) \
    )

// 带 Lambda 初始化的命名实例的类创建器（默认优先级 5）
#define AUTO_REGISTER_CLASS_CREATOR_INSTANCE_WITH_INIT(CLASS_NAME, INSTANCE_NAME, CREATOR_LAMBDA, INIT_LAMBDA) \
    AUTO_REGISTER_CLASS_CREATOR_INSTANCE_WITH_INIT_PRIORITY(CLASS_NAME, INSTANCE_NAME, CREATOR_LAMBDA, INIT_LAMBDA, 5)

// 带 Lambda 初始化的命名实例的类创建器（带优先级）
#define AUTO_REGISTER_CLASS_CREATOR_INSTANCE_WITH_INIT_PRIORITY(CLASS_NAME, INSTANCE_NAME, CREATOR_LAMBDA, INIT_LAMBDA, PRIORITY) \
    _AUTO_REGISTER_IMPL( \
        CLASS_NAME##_##INSTANCE_NAME##_ClassCreatorInitAutoRegister, \
        ::AutoRegister::instance().registerNamedCreatorWithInit<CLASS_NAME>(#INSTANCE_NAME, CREATOR_LAMBDA, INIT_LAMBDA, PRIORITY) \
    )
