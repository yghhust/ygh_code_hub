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
 */
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
 */
#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <functional>
#include <shared_mutex>
#include <typeindex>
#include <unordered_map>
#include <sstream>
#include <mutex>
#include <vector>
#include <algorithm>

// ==================== 注册表项 ====================
class RegEntry {
    using CREATOR = std::function<std::shared_ptr<void>()>;
    using INITIALIZER = std::function<void(std::shared_ptr<void>)>;  // 修改为接受 shared_ptr<void>
private:	
    std::string key_;          // 注册键（类型名+名称）
    CREATOR creator_;          // 实例创建函数
    INITIALIZER initializer_;  // 实例初始化函数（修改为接受 shared_ptr<void>）
    int prior_ = 5;            // 优先级（数值小优先）
    
    bool initialized_ = false;  // 是否已初始化
    std::shared_ptr<void> instance_;  // 实例指针（void*统一存储）

public:
    int priority() { return prior_; } 
    
    // 比较运算符（排序用）
    bool operator < (const RegEntry& other) const {  
        return prior_ < other.prior_;
    }
    
    // 注册信息
    bool regist(const std::string& key, CREATOR creator, INITIALIZER init, int prior = 0) {  
        key_ = key;
        creator_ = creator;
        initializer_ = init;
        prior_ = prior;
        return true;
    }
    
    // 创建实例（惰性初始化）
    std::shared_ptr<void> create() {  
        if ((!instance_) && creator_) instance_ = creator_();
        return instance_;
    }
    
    // 初始化实例
    void init() {  
        if (!initializer_ || initialized_) return;
        if (!instance_ && creator_) instance_ = creator_();
        if (instance_) {
            initializer_(instance_);  // 传递 shared_ptr<void>
            initialized_ = true;
        }
    }
    
    // 获取注册项信息（日志用）
    const std::string info() {  
        std::ostringstream oss;
        oss << "key:" << key_ << ", priority:" << prior_
            << ", hascreator:" << (creator_ != nullptr)
            << ", hasinitializer:" << (initializer_ != nullptr);
        return oss.str();
    }
};

// ==================== 自动注册管理器 ====================
class AutoRegister {
private:
    AutoRegister() = default;
    AutoRegister(const AutoRegister&) = delete;
    AutoRegister& operator=(const AutoRegister&) = delete;
    AutoRegister(AutoRegister&&) = delete;
    AutoRegister& operator=(AutoRegister&&) = delete;

public:    
    static AutoRegister& instance() {  
        static AutoRegister inst;
        return inst;
    }

    template<typename T>    
    using CreatorFunc = std::function<std::shared_ptr<T>()>;
    
    // 修改为接受 shared_ptr<T> 的初始化函数
    template<typename T>   
    using InitFunc = std::function<void(std::shared_ptr<T>)>;
    
    // 注册：无初始化、基本类型
    template<typename T>
    void registerEntry(CreatorFunc<T> creator, int priority = 0) {
        registerEntryImpl<T>("", creator, nullptr, priority);
    }

    // 注册：带初始化、基本类型（使用新的 InitFunc）
    template<typename T>
    void registerEntryWithInit(CreatorFunc<T> creator, InitFunc<T> init, int priority = 0) {
        registerEntryImpl<T>("", creator, init, priority);
    }

    // 注册：无初始化、命名类型
    template<typename T>
    void registerNamedEntry(const std::string& name, CreatorFunc<T> creator, int priority = 0) {
        registerEntryImpl<T>(name, creator, nullptr, priority);
    }

    // 注册：带初始化、命名类型（使用新的 InitFunc）
    template<typename T>
    void registerNamedEntryWithInit(const std::string& name, CreatorFunc<T> creator, 
                                  InitFunc<T> init, int priority = 0) {
        registerEntryImpl<T>(name, creator, init, priority);
    }
    
    // 执行所有初始化（优先级≤10）
    void executeAllInits() {  
        executePriorInits(10);
    }

	/**
	 * @brief 执行指定最大优先级的初始化
	 * @param maxPri 最大优先级（只处理优先级≤此值的注册项）
	 * @note 按优先级升序执行：先创建所有实例，再按顺序初始化
	 */
    void executePriorInits(int maxPri) {  
        logInfo("executePriorInits start, maxPri=", maxPri);
        
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<std::shared_ptr<RegEntry>> ent_vec;
        for (auto& [key, entry] : registry_) {
            if (entry->priority() <= maxPri) ent_vec.emplace_back(entry);
        }

        // 按优先级排序（数值小的优先级高）
        std::sort(ent_vec.begin(), ent_vec.end(),
                 [](const auto& a, const auto& b) { return *a < *b; });

        for (auto it : ent_vec) {  // 创建实例
            logInfo(it->info());
            it->create();
        }
        for (auto it : ent_vec) {  // 初始化实例
            it->init();
        }    
    }

	// 获取实例（惰性初始化）
    template<typename T>
    std::shared_ptr<T> getInstance(const std::string& name = "") {  
        std::string key = makeKey<T>(name);
        logDebug("getInstance called, key=", key);
        
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = registry_.find(key);
        if (it == registry_.end()) {
            logError("No registration for key=", key);
            return nullptr;
        }
        
        auto entry = it->second;
        auto instance = entry->create();
        if (!instance) {
            logError("lazyCreate returned null for key=", key);
            return nullptr;
        }
        entry->init();
        return std::static_pointer_cast<T>(instance);
    }

private:
    // 生成唯一键（类型名+名称）
    template<typename T>
    static std::string makeKey(const std::string& name = "") {  
        auto key = std::string(std::type_index(typeid(T)).name());
        return name.empty() ? key : key + "_" + name;
    }
    
    // 注册实现（使用新的 InitFunc）
    template<typename T>    
    void registerEntryImpl(const std::string& name, CreatorFunc<T> creator, 
                          InitFunc<T> init, int priority = 0) {  
        std::string key = makeKey<T>(name);
        
        // 包装初始化函数：接受 shared_ptr<void> 并转换为 shared_ptr<T>
        std::function<void(std::shared_ptr<void>)> initializer = nullptr;
        if (init) {
            initializer = [init](std::shared_ptr<void> p) {
                auto derived = std::static_pointer_cast<T>(p);
                init(derived);
            };
        }

        auto entry = std::make_shared<RegEntry>();
        entry->regist(key, creator, initializer, priority);

        std::lock_guard<std::mutex> lock(mutex_);
        registry_[key] = std::move(entry);
        logDebug("Registered key=", key, " pri=", priority);
    }  

public:
    // 日志函数
    template<typename... Args>
    void logDebug(Args&&... args) {  
        if (logLevel_ <= LogLevel::DEBUG)
            logImpl("[AutoRegister DEBUG] ", std::forward<Args>(args)...);
    }
    template<typename... Args>
    void logInfo(Args&&... args) {  
        if (logLevel_ <= LogLevel::INFO)
            logImpl("[AutoRegister INFO] ", std::forward<Args>(args)...);
    }
    template<typename... Args>
    void logWarn(Args&&... args) {  
        if (logLevel_ <= LogLevel::WARN)
            logImpl("[AutoRegister WARN] ", std::forward<Args>(args)...);
    }
    template<typename... Args>
    void logError(Args&&... args) { 
        if (logLevel_ <= LogLevel::ERROR)
            logImpl("[AutoRegister ERROR] ", std::forward<Args>(args)...);
    }

    template<typename... Args>
    void logImpl(const char* prefix, Args&&... args) { 
        std::cout << prefix;
        (std::cout << ... << std::forward<Args>(args)) << "\n";
    }

    enum class LogLevel { DEBUG, INFO, WARN, ERROR };  
    LogLevel logLevel_ = LogLevel::DEBUG;  
    
private:       
    std::mutex mutex_;  
    std::unordered_map<std::string, std::shared_ptr<RegEntry>> registry_; 
};

// ==================== 注册宏 ====================
#define _REG_HELPER(NAME, ...) \
    namespace { \
        static struct RegisterHelper_##NAME { \
            RegisterHelper_##NAME() { AutoRegister::instance().__VA_ARGS__; } \
        } s_reg_helper_##NAME; \
    }

// 基础注册（无初始化）
#define AUTO_REG_CLASS(CLS) \
    _REG_HELPER(CLS, registerEntry<CLS>([]() { return std::make_shared<CLS>(); }))

// 带优先级注册（无初始化）
#define AUTO_REG_CLASS_PRI(CLS, PRI) \
    _REG_HELPER(CLS##_PRI##PRI, registerEntry<CLS>([]() { return std::make_shared<CLS>(); }, PRI))

// 带初始化注册（无优先级）- 注意：lambda 现在接受 shared_ptr
#define AUTO_REG_CLASS_INIT(CLS, INIT_LAMBDA) \
    _REG_HELPER(CLS##_INIT, registerEntryWithInit<CLS>([]() { return std::make_shared<CLS>(); }, INIT_LAMBDA))

// 带初始化+优先级注册
#define AUTO_REG_CLASS_INIT_PRI(CLS, INIT_LAMBDA, PRI) \
    _REG_HELPER(CLS##_INIT_PRI##PRI, registerEntryWithInit<CLS>([]() { return std::make_shared<CLS>(); }, INIT_LAMBDA, PRI))

// 命名注册（无初始化）
#define AUTO_REG_NAMED(CLS, NAME) \
    _REG_HELPER(CLS##_NAMED_##NAME, registerNamedEntry<CLS>(#NAME, []() { return std::make_shared<CLS>(); }))

// 命名+优先级注册（无初始化）
#define AUTO_REG_NAMED_PRI(CLS, NAME, PRI) \
    _REG_HELPER(CLS##_NAMED_##NAME##_PRI##PRI, registerNamedEntry<CLS>(#NAME, []() { return std::make_shared<CLS>(); }, PRI))

// 命名+初始化注册
#define AUTO_REG_NAMED_INIT(CLS, NAME, INIT_LAMBDA) \
    _REG_HELPER(CLS##_NAMED_##NAME##_INIT, registerNamedEntryWithInit<CLS>(#NAME, []() { return std::make_shared<CLS>(); }, INIT_LAMBDA))

// 命名+初始化+优先级注册
#define AUTO_REG_NAMED_INIT_PRI(CLS, NAME, INIT_LAMBDA, PRI) \
    _REG_HELPER(CLS##_NAMED_##NAME##_INIT_PRI##PRI, registerNamedEntryWithInit<CLS>(#NAME, []() { return std::make_shared<CLS>(); }, INIT_LAMBDA, PRI))

// 自定义创建函数注册（无初始化）
#define AUTO_REG_CREATOR(CLS, CREATOR_LAMBDA) \
    _REG_HELPER(CLS##_CREATOR, registerEntry<CLS>(CREATOR_LAMBDA))

// 自定义创建函数+优先级注册
#define AUTO_REG_CREATOR_PRI(CLS, CREATOR_LAMBDA, PRI) \
    _REG_HELPER(CLS##_CREATOR_PRI##PRI, registerEntry<CLS>(CREATOR_LAMBDA, PRI))

// 自定义创建+初始化函数注册
#define AUTO_REG_CREATOR_INIT(CLS, CREATOR_LAMBDA, INIT_LAMBDA) \
    _REG_HELPER(CLS##_CREATOR_INIT, registerEntryWithInit<CLS>(CREATOR_LAMBDA, INIT_LAMBDA))

// 自定义创建+初始化+优先级注册
#define AUTO_REG_CREATOR_INIT_PRI(CLS, CREATOR_LAMBDA, INIT_LAMBDA, PRI) \
    _REG_HELPER(CLS##_CREATOR_INIT_PRI##PRI, registerEntryWithInit<CLS>(CREATOR_LAMBDA, INIT_LAMBDA, PRI))

// 自定义创建函数+命名注册
#define AUTO_REG_CREATOR_NAMED(CLS, NAME, CREATOR_LAMBDA) \
    _REG_HELPER(CLS##_CREATOR_NAMED_##NAME, registerNamedEntry<CLS>(#NAME, CREATOR_LAMBDA))

// 自定义创建+命名+优先级注册
#define AUTO_REG_CREATOR_NAMED_PRI(CLS, NAME, CREATOR_LAMBDA, PRI) \
    _REG_HELPER(CLS##_CREATOR_NAMED_##NAME##_PRI##PRI, registerNamedEntry<CLS>(#NAME, CREATOR_LAMBDA, PRI))

// 自定义创建+命名+初始化注册
#define AUTO_REG_CREATOR_NAMED_INIT(CLS, NAME, CREATOR_LAMBDA, INIT_LAMBDA) \
    _REG_HELPER(CLS##_CREATOR_NAMED_##NAME##_INIT, registerNamedEntryWithInit<CLS>(#NAME, CREATOR_LAMBDA, INIT_LAMBDA))

// 自定义创建+命名+初始化+优先级注册
#define AUTO_REG_CREATOR_NAMED_INIT_PRI(CLS, NAME, CREATOR_LAMBDA, INIT_LAMBDA, PRI) \
    _REG_HELPER(CLS##_CREATOR_NAMED_##NAME##_INIT_PRI##PRI, registerNamedEntryWithInit<CLS>(#NAME, CREATOR_LAMBDA, INIT_LAMBDA, PRI))

// 成员函数初始化注册（无优先级）- 注意：lambda 现在接受 shared_ptr
#define AUTO_REG_CLASS_INITFUNC(CLS, INIT_MEMBER_FUNC) \
    _REG_HELPER(CLS##_INITFUNC, registerEntryWithInit<CLS>([]() { return std::make_shared<CLS>(); }, [](std::shared_ptr<CLS> p) { p->INIT_MEMBER_FUNC(); }))

// 成员函数初始化+优先级注册
#define AUTO_REG_CLASS_INITFUNC_PRI(CLS, INIT_MEMBER_FUNC, PRI) \
    _REG_HELPER(CLS##_INITFUNC_PRI##PRI, registerEntryWithInit<CLS>([]() { return std::make_shared<CLS>(); }, [](std::shared_ptr<CLS> p) { p->INIT_MEMBER_FUNC(); }, PRI))

// 命名+成员函数初始化注册
#define AUTO_REG_NAMED_INITFUNC(CLS, NAME, INIT_MEMBER_FUNC) \
    _REG_HELPER(CLS##_NAMED_##NAME##_INITFUNC, registerNamedEntryWithInit<CLS>(#NAME, []() { return std::make_shared<CLS>(); }, [](std::shared_ptr<CLS> p) { p->INIT_MEMBER_FUNC(); }))

// 命名+成员函数初始化+优先级注册
#define AUTO_REG_NAMED_INITFUNC_PRI(CLS, NAME, INIT_MEMBER_FUNC, PRI) \
    _REG_HELPER(CLS##_NAMED_##NAME##_INITFUNC_PRI##PRI, registerNamedEntryWithInit<CLS>(#NAME, []() { return std::make_shared<CLS>(); }, [](std::shared_ptr<CLS> p) { p->INIT_MEMBER_FUNC(); }, PRI))

// 自定义创建+成员函数初始化注册
#define AUTO_REG_CREATOR_INITFUNC(CLS, CREATOR_LAMBDA, INIT_MEMBER_FUNC) \
    _REG_HELPER(CLS##_CREATOR_INITFUNC, registerEntryWithInit<CLS>(CREATOR_LAMBDA, [](std::shared_ptr<CLS> p) { p->INIT_MEMBER_FUNC(); }))

// 自定义创建+成员函数初始化+优先级注册
#define AUTO_REG_CREATOR_INITFUNC_PRI(CLS, CREATOR_LAMBDA, INIT_MEMBER_FUNC, PRI) \
    _REG_HELPER(CLS##_CREATOR_INITFUNC_PRI##PRI, registerEntryWithInit<CLS>(CREATOR_LAMBDA, [](std::shared_ptr<CLS> p) { p->INIT_MEMBER_FUNC(); }, PRI))

// 自定义创建+命名+成员函数初始化注册
#define AUTO_REG_CREATOR_NAMED_INITFUNC(CLS, NAME, CREATOR_LAMBDA, INIT_MEMBER_FUNC) \
    _REG_HELPER(CLS##_CREATOR_NAMED_##NAME##_INITFUNC, registerNamedEntryWithInit<CLS>(#NAME, CREATOR_LAMBDA, [](std::shared_ptr<CLS> p) { p->INIT_MEMBER_FUNC(); }))

// 自定义创建+命名+成员函数初始化+优先级注册
#define AUTO_REG_CREATOR_NAMED_INITFUNC_PRI(CLS, NAME, CREATOR_LAMBDA, INIT_MEMBER_FUNC, PRI) \
    _REG_HELPER(CLS##_CREATOR_NAMED_##NAME##_INITFUNC_PRI##PRI, registerNamedEntryWithInit<CLS>(#NAME, CREATOR_LAMBDA, [](std::shared_ptr<CLS> p) { p->INIT_MEMBER_FUNC(); }, PRI))
