/**
 * @file example.cpp
 * @brief 演示 auto_register.h 中所有宏的用法
 */

#include "auto_register.h"
#include <iostream>
#include <string>

// --- 测试类 ---

// 1. 基础类，无初始化
class SimpleService {
public:
    std::string name = "SimpleService";
    void sayHello() {
        std::cout << "[SimpleService] Hello from " << name << std::endl;
    }
};

// 2. 带初始化函数的类
class ConfiguredService {
public:
    int value = 0;
    std::string config = "default";
    
    void init() {
        value = 42;
        config = "initialized_by_member_func";
        std::cout << "[ConfiguredService] Initialized by member function 'init()'. Value: " << value << std::endl;
    }
    
    void show() {
        std::cout << "[ConfiguredService] Value: " << value << ", Config: " << config << std::endl;
    }
};

// 3. 使用 Lambda 初始化的类
class LambdaInitializedService {
public:
    double factor = 1.0;
    std::string mode = "off";
    void setup() { std::cout << "[LambdaInitializedService] Factor: " << factor << ", Mode: " << mode << std::endl; }
};

// 4. 命名实例类
class DatabaseConnection {
public:
    std::string connectionString;
    bool isConnected = false;
    void connect() { isConnected = true; std::cout << "[DatabaseConnection] Connected to: " << connectionString << std::endl; }
};

// 5. 使用创建器的类
class ComplexObject {
public:
    int id;
    std::string type;
    ComplexObject(int i, const std::string& t) : id(i), type(t) {}
    void describe() {
        std::cout << "[ComplexObject] ID: " << id << ", Type: " << type << std::endl;
    }
};

// --- 注册宏 ---
// 1. 基础类自动注册
AUTO_REG_CLASS(SimpleService)

// 2. 带成员函数初始化的类注册
AUTO_REG_CLASS_INITFUNC(ConfiguredService, init)

// 3. 带 Lambda 初始化的类注册
AUTO_REG_CLASS_INIT(LambdaInitializedService, [](LambdaInitializedService& s) {
    s.factor = 3.14;
    s.mode = "active";
})

// 4. 命名类实例注册
AUTO_REG_NAMED(DatabaseConnection, PrimaryDB)

// 5. 带成员函数初始化的命名实例
AUTO_REG_NAMED_INITFUNC(DatabaseConnection, SecondaryDB, connect)

// 6. 带 Lambda 初始化的命名实例
AUTO_REG_NAMED_INIT(DatabaseConnection, ReadReplica, [](DatabaseConnection& db) {
    db.connectionString = "jdbc:mysql://replica.host/db";
    db.isConnected = true;
})

// 7. 类创建器注册 (Lambda)
AUTO_REG_CREATOR(ComplexObject, []() {
    return std::make_shared<ComplexObject>(100, "CreatorLambda");
})

// 8. 类创建器带成员函数初始化
AUTO_REG_CREATOR_INITFUNC(ComplexObject, []() {
    return std::make_shared<ComplexObject>(200, "CreatorInitFunc");
}, describe)

// 9. 类创建器带 Lambda 初始化
AUTO_REG_CREATOR_INIT(ComplexObject, []() {
    return std::make_shared<ComplexObject>(300, "CreatorLambdaInit");
}, [](ComplexObject& obj) {
    obj.id += 1000;
})

// 10. 命名实例的类创建器
AUTO_REG_CREATOR_NAMED(ComplexObject, InstanceAlpha, []() {
    return std::make_shared<ComplexObject>(400, "InstAlpha");
})

// 11. 命名实例的类创建器带成员函数初始化
AUTO_REG_CREATOR_NAMED_INITFUNC(ComplexObject, InstanceBeta, []() {
    return std::make_shared<ComplexObject>(500, "InstBeta");
}, describe)

// 12. 命名实例的类创建器带 Lambda 初始化
AUTO_REG_CREATOR_NAMED_INIT(ComplexObject, InstanceGamma, []() {
    return std::make_shared<ComplexObject>(600, "InstGamma");
}, [](ComplexObject& obj) {
    obj.type = "Modified";
})

// --- 主函数 ---

int example1() {
    std::cout << "========================================" << std::endl;
    std::cout << "   AutoRegister Framework Demo         " << std::endl;
    std::cout << "========================================" << std::endl;

    // 执行所有初始化 (会触发带初始化的注册的 init 函数)
    std::cout << "--- Executing all initializers ---" << std::endl;
    AutoRegister::instance().executeAllInits();
    std::cout << std::endl;

    // 获取并使用实例
    std::cout << "--- Getting and using instances ---" << std::endl;

    auto simple = AutoRegister::instance().getInstance<SimpleService>();
    if (simple) simple->sayHello();

    auto configured = AutoRegister::instance().getInstance<ConfiguredService>();
    if (configured) configured->show();

    auto lambdaInit = AutoRegister::instance().getInstance<LambdaInitializedService>();
    if (lambdaInit) lambdaInit->setup();

    auto primaryDb = AutoRegister::instance().getInstance<DatabaseConnection>("PrimaryDB");
    if (primaryDb) { std::cout << "[Main] PrimaryDB 创建，连接中..." << std::endl; primaryDb->connect(); }

    auto secondaryDb = AutoRegister::instance().getInstance<DatabaseConnection>("SecondaryDB");
    if (secondaryDb) std::cout << "[Main] SecondaryDB 状态: " << (secondaryDb->isConnected ? "已连接" : "未连接") << std::endl;

    auto replicaDb = AutoRegister::instance().getInstance<DatabaseConnection>("ReadReplica");
    if (replicaDb) std::cout << "[Main] ReplicaDB 连接至: " << replicaDb->connectionString << std::endl;

    // 命名实例访问
    auto alpha = AutoRegister::instance().getInstance<ComplexObject>("InstanceAlpha");
    if (alpha) alpha->describe();

    auto beta = AutoRegister::instance().getInstance<ComplexObject>("InstanceBeta");
    if (beta) beta->describe();

    auto gamma = AutoRegister::instance().getInstance<ComplexObject>("InstanceGamma");
    if (gamma) gamma->describe();

    return 0;
}
