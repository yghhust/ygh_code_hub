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

class AutoRegister;

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
    std::unordered_map<std::string, std::function<std::shared_ptr<void>()>> creators_;
    std::unordered_map<std::string, std::shared_ptr<void>> instances_;
    std::unordered_map<int, std::vector<std::function<void()>>> init_queues_;

    std::string makeFullName(const std::string& class_name, const std::string& instance_name) {
        return class_name + "#" + instance_name;
    }

    void addToInitQueue(const std::string& class_name, int priority, std::function<void()> init_func) {
        init_queues_[priority].push_back(std::move(init_func));
    }

public:
    void registerCreator(const std::string& class_name,
                        std::function<std::shared_ptr<void>()> creator_lambda,
                        int priority = 5) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (creators_.count(class_name)) {
            std::cerr << "[AutoRegister WARNING] Creator already exists for '" 
                      << class_name << "', overwriting." << std::endl;
        }
        priority = std::clamp(priority, 0, 10);
        creators_[class_name] = std::move(creator_lambda);
        addToInitQueue(class_name, priority, [this, class_name]() {
            lazyCreateInstance<void>(class_name);
        });
        std::cout << "[AutoRegister INFO] Lambda creator registered for '" 
                  << class_name << "' with priority " << priority << std::endl;
    }

    template<typename T>
    void registerCreatorWithInit(const std::string& class_name,
                                 std::function<std::shared_ptr<T>()> creator_lambda,
                                 std::function<void(T&)> init_lambda,
                                 int priority = 5) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (creators_.count(class_name)) {
            std::cerr << "[AutoRegister WARNING] Creator already exists for '" 
                      << class_name << "', overwriting." << std::endl;
        }
        priority = std::clamp(priority, 0, 10);
        creators_[class_name] = [class_name, creator_lambda, init_lambda]() -> std::shared_ptr<void> {
            try {
                auto instance = creator_lambda();
                if (instance) init_lambda(*instance);
                return instance;
            } catch (const std::exception& e) {
                std::cerr << "[AutoRegister ERROR] Creator with init failed for '" 
                          << class_name << "': " << e.what() << std::endl;
                return nullptr;
            }
        };
        addToInitQueue(class_name, priority, [this, class_name]() {
            lazyCreateInstance<T>(class_name);
        });
        std::cout << "[AutoRegister INFO] Lambda creator with init registered for '" 
                  << class_name << "' with priority " << priority << std::endl;
    }

    void registerNamedCreator(const std::string& class_name,
                             const std::string& instance_name,
                             std::function<std::shared_ptr<void>()> creator_lambda,
                             int priority = 5) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::string full_name = makeFullName(class_name, instance_name);
        if (creators_.count(full_name)) {
            std::cerr << "[AutoRegister WARNING] Named creator already exists for '" 
                      << full_name << "', overwriting." << std::endl;
        }
        priority = std::clamp(priority, 0, 10);
        creators_[full_name] = std::move(creator_lambda);
        addToInitQueue(full_name, priority, [this, full_name, instance_name]() {
            lazyCreateNamedInstance<void>(full_name, instance_name);
        });
        std::cout << "[AutoRegister INFO] Lambda named creator registered for '" 
                  << full_name << "' with priority " << priority << std::endl;
    }

    template<typename T>
    void registerNamedCreatorWithInit(const std::string& class_name,
                                      const std::string& instance_name,
                                      std::function<std::shared_ptr<T>()> creator_lambda,
                                      std::function<void(T&)> init_lambda,
                                      int priority = 5) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::string full_name = makeFullName(class_name, instance_name);
        if (creators_.count(full_name)) {
            std::cerr << "[AutoRegister WARNING] Named creator already exists for '" 
                      << full_name << "', overwriting." << std::endl;
        }
        priority = std::clamp(priority, 0, 10);
        creators_[full_name] = [full_name, creator_lambda, init_lambda]() -> std::shared_ptr<void> {
            try {
                auto instance = creator_lambda();
                if (instance) init_lambda(*instance);
                return instance;
            } catch (const std::exception& e) {
                std::cerr << "[AutoRegister ERROR] Named creator with init failed for '" 
                          << full_name << "': " << e.what() << std::endl;
                return nullptr;
            }
        };
        addToInitQueue(full_name, priority, [this, full_name, instance_name]() {
            lazyCreateNamedInstance<T>(full_name, instance_name);
        });
        std::cout << "[AutoRegister INFO] Lambda named creator with init registered for '" 
                  << full_name << "' with priority " << priority << std::endl;
    }

    template<typename T>
    void registerClass(const std::string& class_name, int priority = 5) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (creators_.count(class_name)) return;
        priority = std::clamp(priority, 0, 10);
        creators_[class_name] = [class_name]() -> std::shared_ptr<void> {
            try { return std::make_shared<T>(); }
            catch (const std::exception& e) {
                std::cerr << "[AutoRegister ERROR] Creator failed for " << class_name 
                          << ": " << e.what() << std::endl;
                return nullptr;
            }
        };
        addToInitQueue(class_name, priority, [this, class_name]() {
            lazyCreateInstance<T>(class_name);
        });
    }

    template<typename T>
    void registerClassWithInit(const std::string& class_name, 
                               std::function<void(T&)> init_func = nullptr,
                               int priority = 5) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (creators_.count(class_name)) return;
        priority = std::clamp(priority, 0, 10);
        creators_[class_name] = [class_name, init_func]() -> std::shared_ptr<void> {
            try {
                auto instance = std::make_shared<T>();
                if (init_func) init_func(*instance);
                return instance;
            } catch (const std::exception& e) {
                std::cerr << "[AutoRegister ERROR] Creator with init failed for " << class_name 
                          << ": " << e.what() << std::endl;
                return nullptr;
            }
        };
        addToInitQueue(class_name, priority, [this, class_name, init_func]() {
            lazyCreateInstanceWithInit<T>(class_name, init_func);
        });
    }

    template<typename T>
    void registerNamedInstance(const std::string& instance_name, 
                              const std::string& class_name, 
                              int priority = 5) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::string full_name = makeFullName(class_name, instance_name);
        if (creators_.count(full_name)) return;
        priority = std::clamp(priority, 0, 10);
        creators_[full_name] = [full_name, instance_name]() -> std::shared_ptr<void> {
            try { return std::make_shared<T>(); }
            catch (const std::exception& e) {
                std::cerr << "[AutoRegister ERROR] Creator failed for named instance " << full_name 
                          << ": " << e.what() << std::endl;
                return nullptr;
            }
        };
        addToInitQueue(full_name, priority, [this, full_name, instance_name]() {
            lazyCreateNamedInstance<T>(full_name, instance_name);
        });
    }

    template<typename T>
    void registerNamedInstanceWithInit(const std::string& instance_name, 
                                       const std::string& class_name,
                                       std::function<void(T&)> init_func = nullptr,
                                       int priority = 5) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::string full_name = makeFullName(class_name, instance_name);
        if (creators_.count(full_name)) return;
        priority = std::clamp(priority, 0, 10);
        creators_[full_name] = [full_name, instance_name, init_func]() -> std::shared_ptr<void> {
            try {
                auto instance = std::make_shared<T>();
                if (init_func) init_func(*instance);
                return instance;
            } catch (const std::exception& e) {
                std::cerr << "[AutoRegister ERROR] Creator with init failed for named instance " << full_name 
                          << ": " << e.what() << std::endl;
                return nullptr;
            }
        };
        addToInitQueue(full_name, priority, [this, full_name, instance_name, init_func]() {
            lazyCreateNamedInstanceWithInit<T>(full_name, instance_name, init_func);
        });
    }

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

    void executeInitsAtPriority(int priority) {
        priority = std::clamp(priority, 0, 10);
        auto inits = collectInitsAtPriority(priority);
        executeAllCollectedInits(std::move(inits), 
            "[AutoRegister::Init] Executing priority " + std::to_string(priority) + " initializers");
    }

    template<typename T>
    std::shared_ptr<T> getInstance(const std::string& class_name) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = instances_.find(class_name);
        if (it != instances_.end()) {
            return std::static_pointer_cast<T>(it->second);
        }
        return lazyCreateInstance<T>(class_name);
    }

    template<typename T>
    std::shared_ptr<T> getInstanceByName(const std::string& instance_name, 
                                        const std::string& class_name) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::string full_name = makeFullName(class_name, instance_name);
        auto it = instances_.find(full_name);
        if (it != instances_.end()) {
            return std::static_pointer_cast<T>(it->second);
        }
        return lazyCreateNamedInstance<T>(full_name, instance_name);
    }

    template<typename T>
    bool hasInstance(const std::string& class_name) {
        std::lock_guard<std::mutex> lock(mutex_);
        return instances_.find(class_name) != instances_.end();
    }

    template<typename T>
    bool hasNamedInstance(const std::string& instance_name, 
                         const std::string& class_name) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::string full_name = makeFullName(class_name, instance_name);
        return instances_.find(full_name) != instances_.end();
    }

    template<typename T>
    std::shared_ptr<T> createTempInstance() {
        return std::make_shared<T>();
    }

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

    void printInitQueues() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::cout << "\n[AutoRegister::Debug] Init queues status:" << std::endl;
        for (int priority = 0; priority <= 10; ++priority) {
            auto it = init_queues_.find(priority);
            if (it != init_queues_.end() && !it->second.empty()) {
                std::cout << "  Priority " << priority << ": " << it->second.size() << " items" << std::endl;
            }
        }
        std::cout << std::endl;
    }

    void printCreators() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::cout << "\n[AutoRegister::Debug] Creators registered (" << creators_.size() << "):" << std::endl;
        for (const auto& pair : creators_) {
            std::cout << "  - Key: " << pair.first << std::endl;
        }
        std::cout << std::endl;
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        instances_.clear();
        creators_.clear();
        init_queues_.clear();
    }

    size_t getInstanceCount() const { 
        std::lock_guard<std::mutex> lock(mutex_);
        return instances_.size(); 
    }
    
    size_t getRegisteredCount() const { 
        std::lock_guard<std::mutex> lock(mutex_);
        return creators_.size(); 
    }

    template<typename T>
    std::shared_ptr<T> forceInit(const std::string& class_name) {
        return lazyCreateInstance<T>(class_name);
    }

    template<typename T>
    std::shared_ptr<T> forceInitNamed(const std::string& instance_name,
                                       const std::string& class_name) {
        std::string full_name = makeFullName(class_name, instance_name);
        return lazyCreateNamedInstance<T>(full_name, instance_name);
    }

    template<typename T>
    std::shared_ptr<T> lazyCreateInstance(const std::string& class_name) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = instances_.find(class_name);
            if (it != instances_.end()) {
                return std::static_pointer_cast<T>(it->second);
            }
        }
        std::shared_ptr<void> instance;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = creators_.find(class_name);
            if (it == creators_.end()) {
                std::cerr << "[AutoRegister ERROR] No creator found for " << class_name << std::endl;
                return nullptr;
            }
            try { instance = it->second(); }
            catch (const std::exception& e) {
                std::cerr << "[AutoRegister ERROR] Failed to create instance " << class_name << ": " << e.what() << std::endl;
                return nullptr;
            }
        }
        if (instance) {
            std::lock_guard<std::mutex> lock(mutex_);
            instances_[class_name] = instance;
        }
        return std::static_pointer_cast<T>(instance);
    }

    template<typename T>
    std::shared_ptr<T> lazyCreateInstanceWithInit(const std::string& class_name, 
                                                     std::function<void(T&)> init_func) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = instances_.find(class_name);
            if (it != instances_.end()) {
                return std::static_pointer_cast<T>(it->second);
            }
        }
        std::shared_ptr<void> instance;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = creators_.find(class_name);
            if (it == creators_.end()) {
                std::cerr << "[AutoRegister ERROR] No creator found for " << class_name << std::endl;
                return nullptr;
            }
            try { instance = it->second(); }
            catch (const std::exception& e) {
                std::cerr << "[AutoRegister ERROR] Failed to create instance " << class_name << ": " << e.what() << std::endl;
                return nullptr;
            }
        }
        if (instance) {
            std::lock_guard<std::mutex> lock(mutex_);
            instances_[class_name] = instance;
        }
        return std::static_pointer_cast<T>(instance);
    }

    template<typename T>
    std::shared_ptr<T> lazyCreateNamedInstance(const std::string& full_name, 
                                                 const std::string& instance_name) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = instances_.find(full_name);
            if (it != instances_.end()) {
                return std::static_pointer_cast<T>(it->second);
            }
        }
        std::shared_ptr<void> instance;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = creators_.find(full_name);
            if (it == creators_.end()) {
                std::cerr << "[AutoRegister ERROR] No creator found for " << full_name << std::endl;
                return nullptr;
            }
            try { instance = it->second(); }
            catch (const std::exception& e) {
                std::cerr << "[AutoRegister ERROR] Failed to create named instance " << full_name << ": " << e.what() << std::endl;
                return nullptr;
            }
        }
        if (instance) {
            std::lock_guard<std::mutex> lock(mutex_);
            instances_[full_name] = instance;
        }
        return std::static_pointer_cast<T>(instance);
    }

    template<typename T>
    std::shared_ptr<T> lazyCreateNamedInstanceWithInit(const std::string& full_name, 
                                                          const std::string& instance_name,
                                                          std::function<void(T&)> init_func) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = instances_.find(full_name);
            if (it != instances_.end()) {
                return std::static_pointer_cast<T>(it->second);
            }
        }
        std::shared_ptr<void> instance;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = creators_.find(full_name);
            if (it == creators_.end()) {
                std::cerr << "[AutoRegister ERROR] No creator found for " << full_name << std::endl;
                return nullptr;
            }
            try { instance = it->second(); }
            catch (const std::exception& e) {
                std::cerr << "[AutoRegister ERROR] Failed to create named instance " << full_name << ": " << e.what() << std::endl;
                return nullptr;
            }
        }
        if (instance) {
            std::lock_guard<std::mutex> lock(mutex_);
            instances_[full_name] = instance;
        }
        return std::static_pointer_cast<T>(instance);
    }

    std::vector<std::function<void()>> collectInitsUpToPriority(int max_priority) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<std::function<void()>> all_inits;
        for (int priority = 0; priority <= max_priority; ++priority) {
            auto it = init_queues_.find(priority);
            if (it != init_queues_.end()) {
                all_inits.insert(all_inits.end(), it->second.begin(), it->second.end());
            }
        }
        return all_inits;
    }

    std::vector<std::function<void()>> collectInitsAtPriority(int priority) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<std::function<void()>> inits;
        auto it = init_queues_.find(priority);
        if (it != init_queues_.end()) {
            inits = it->second;
        }
        return inits;
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

inline void registerCreator(const std::string& class_name,
                           std::function<std::shared_ptr<void>()> creator_lambda,
                           int priority = 5) {
    AutoRegister::instance().registerCreator(class_name, std::move(creator_lambda), priority);
}

template<typename T>
void registerCreator(const std::string& class_name,
                    std::function<std::shared_ptr<T>()> creator_lambda,
                    int priority = 5) {
    AutoRegister::instance().registerCreatorWithInit<T>(
        class_name,
        std::move(creator_lambda),
        [](T&) {}
    );
}

template<typename T>
void registerCreatorWithInit(const std::string& class_name,
                             std::function<std::shared_ptr<T>()> creator_lambda,
                             std::function<void(T&)> init_lambda,
                             int priority = 5) {
    AutoRegister::instance().registerCreatorWithInit<T>(
        class_name,
        std::move(creator_lambda),
        std::move(init_lambda),
        priority
    );
}

inline void registerNamedCreator(const std::string& class_name,
                                  const std::string& instance_name,
                                  std::function<std::shared_ptr<void>()> creator_lambda,
                                  int priority = 5) {
    AutoRegister::instance().registerNamedCreator(
        class_name, instance_name, std::move(creator_lambda), priority
    );
}

template<typename T>
void registerNamedCreator(const std::string& class_name,
                          const std::string& instance_name,
                          std::function<std::shared_ptr<T>()> creator_lambda,
                          int priority = 5) {
    AutoRegister::instance().registerNamedCreatorWithInit<T>(
        class_name, instance_name,
        std::move(creator_lambda),
        [](T&) {},
        priority
    );
}

template<typename T>
void registerNamedCreatorWithInit(const std::string& class_name,
                                   const std::string& instance_name,
                                   std::function<std::shared_ptr<T>()> creator_lambda,
                                   std::function<void(T&)> init_lambda,
                                   int priority = 5) {
    AutoRegister::instance().registerNamedCreatorWithInit<T>(
        class_name, instance_name,
        std::move(creator_lambda),
        std::move(init_lambda),
        priority
    );
}

// ==================== 统一宏实现 ====================

#define _AUTO_REGISTER_IMPL(UNIQUE_NAME, REGISTER_CALL) \
    static auto UNIQUE_NAME = []() -> auto& { \
        struct Helper { \
            Helper() { \
                REGISTER_CALL; \
            } \
        }; \
        static Helper instance; \
        return instance; \
    }()

#define _AUTO_REGISTER_LAMBDA_IMPL(UNIQUE_NAME, REGISTER_CALL) \
    static auto UNIQUE_NAME = []() -> auto& { \
        struct Helper { \
            Helper() { \
                REGISTER_CALL; \
            } \
        }; \
        static Helper instance; \
        return instance; \
    }()

// 原有宏统一用 _AUTO_REGISTER_IMPL
#define AUTO_REGISTER_CLASS(CLASS_NAME) \
    AUTO_REGISTER_CLASS_PRIORITY(CLASS_NAME, 5)

#define AUTO_REGISTER_CLASS_PRIORITY(CLASS_NAME, PRIORITY) \
    _AUTO_REGISTER_IMPL( \
        CLASS_NAME##_AutoRegister, \
        ::registerClass<CLASS_NAME>(typeid(CLASS_NAME).name(), PRIORITY) \
    )

#define AUTO_REGISTER_CLASS_WITH_INIT(CLASS_NAME, INIT_MEMBER_FUNC) \
    AUTO_REGISTER_CLASS_WITH_INIT_PRIORITY(CLASS_NAME, INIT_MEMBER_FUNC, 5)

#define AUTO_REGISTER_CLASS_WITH_INIT_PRIORITY(CLASS_NAME, INIT_MEMBER_FUNC, PRIORITY) \
    _AUTO_REGISTER_IMPL( \
        CLASS_NAME##_AutoRegister, \
        ::registerClassWithInit<CLASS_NAME>( \
            typeid(CLASS_NAME).name(), [](CLASS_NAME& obj) { obj.INIT_MEMBER_FUNC(); }, PRIORITY) \
    )

#define AUTO_REGISTER_CLASS_WITH_LAMBDA(CLASS_NAME, LAMBDA_INIT) \
    AUTO_REGISTER_CLASS_WITH_LAMBDA_PRIORITY(CLASS_NAME, LAMBDA_INIT, 5)

#define AUTO_REGISTER_CLASS_WITH_LAMBDA_PRIORITY(CLASS_NAME, LAMBDA_INIT, PRIORITY) \
    _AUTO_REGISTER_IMPL( \
        CLASS_NAME##_AutoRegister, \
        ::registerClassWithInit<CLASS_NAME>( \
            typeid(CLASS_NAME).name(), LAMBDA_INIT, PRIORITY) \
    )

#define AUTO_REGISTER_NAMED_INSTANCE(CLASS_NAME, INSTANCE_NAME) \
    _AUTO_REGISTER_IMPL( \
        CLASS_NAME##_##INSTANCE_NAME##_AutoRegister, \
        ::registerNamedInstance<CLASS_NAME>(INSTANCE_NAME) \
    )

#define AUTO_REGISTER_NAMED_INSTANCE_WITH_INIT(CLASS_NAME, INSTANCE_NAME, INIT_MEMBER_FUNC) \
    AUTO_REGISTER_NAMED_INSTANCE_WITH_INIT_PRIORITY(CLASS_NAME, INSTANCE_NAME, INIT_MEMBER_FUNC, 5)

#define AUTO_REGISTER_NAMED_INSTANCE_WITH_INIT_PRIORITY(CLASS_NAME, INSTANCE_NAME, INIT_MEMBER_FUNC, PRIORITY) \
    _AUTO_REGISTER_IMPL( \
        CLASS_NAME##_##INSTANCE_NAME##_AutoRegister, \
        ::registerNamedInstanceWithInit<CLASS_NAME>(INSTANCE_NAME, \
            typeid(CLASS_NAME).name(), [](CLASS_NAME& obj) { obj.INIT_MEMBER_FUNC(); }, PRIORITY) \
    )

#define AUTO_REGISTER_NAMED_INSTANCE_WITH_LAMBDA(CLASS_NAME, INSTANCE_NAME, LAMBDA_INIT) \
    AUTO_REGISTER_NAMED_INSTANCE_WITH_LAMBDA_PRIORITY(CLASS_NAME, INSTANCE_NAME, LAMBDA_INIT, 5)

#define AUTO_REGISTER_NAMED_INSTANCE_WITH_LAMBDA_PRIORITY(CLASS_NAME, INSTANCE_NAME, LAMBDA_INIT, PRIORITY) \
    _AUTO_REGISTER_IMPL( \
        CLASS_NAME##_##INSTANCE_NAME##_AutoRegister, \
        ::registerNamedInstanceWithInit<CLASS_NAME>(INSTANCE_NAME, \
            typeid(CLASS_NAME).name(), LAMBDA_INIT, PRIORITY) \
    )

// Lambda 定制注册宏用 _AUTO_REGISTER_LAMBDA_IMPL
#define AUTO_REGISTER_LAMBDA_CREATOR(CLASS_NAME, CREATOR_LAMBDA) \
    _AUTO_REGISTER_LAMBDA_IMPL( \
        CLASS_NAME##_LambdaAutoRegister, \
        ::registerCreator(#CLASS_NAME, CREATOR_LAMBDA) \
    )

#define AUTO_REGISTER_LAMBDA_CREATOR_WITH_INIT(CLASS_NAME, CREATOR_LAMBDA, INIT_LAMBDA) \
    _AUTO_REGISTER_LAMBDA_IMPL( \
        CLASS_NAME##_LambdaInitAutoRegister, \
        ::registerCreatorWithInit<CLASS_NAME>(#CLASS_NAME, CREATOR_LAMBDA, INIT_LAMBDA) \
    )

#define AUTO_REGISTER_LAMBDA_NAMED_CREATOR(CLASS_NAME, INSTANCE_NAME, CREATOR_LAMBDA) \
    _AUTO_REGISTER_LAMBDA_IMPL( \
        CLASS_NAME##_##INSTANCE_NAME##_LambdaAutoRegister, \
        ::registerNamedCreator(#CLASS_NAME, #INSTANCE_NAME, CREATOR_LAMBDA) \
    )

#define AUTO_REGISTER_LAMBDA_NAMED_CREATOR_WITH_INIT(CLASS_NAME, INSTANCE_NAME, CREATOR_LAMBDA, INIT_LAMBDA) \
    _AUTO_REGISTER_LAMBDA_IMPL( \
        CLASS_NAME##_##INSTANCE_NAME##_LambdaInitAutoRegister, \
        ::registerNamedCreatorWithInit<CLASS_NAME>(#INSTANCE_NAME, CREATOR_LAMBDA, INIT_LAMBDA) \
    )

