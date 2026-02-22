/**
 * @file example.cpp
 * @brief 演示 auto_register.h 中所有宏的用法
 */

#include "auto_register.h"
#include <iostream>
#include <string>

// --- 测试类定义 ---

// 1. 基础类，无初始化
class SimpleService {
public:
    std::string name = "SimpleService_Default";
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
    
    void setup() {
        std::cout << "[LambdaInitializedService] Setup complete. Factor: " << factor << ", Mode: " << mode << std::endl;
    }
};

// 4. 命名实例类
class DatabaseConnection {
public:
    std::string connectionString;
    bool isConnected = false;
    
    void connect() {
        isConnected = true;
        std::cout << "[DatabaseConnection] Connected to: " << connectionString << std::endl;
    }
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


// --- 使用宏进行注册 ---

// 1. 基础类自动注册
AUTO_REGISTER_CLASS(SimpleService);

// 2. 带成员函数初始化的类注册
AUTO_REGISTER_CLASS_WITH_INITFUNC(ConfiguredService, init);

// 3. 带 Lambda 初始化的类注册
AUTO_REGISTER_CLASS_WITH_INIT(LambdaInitializedService, [](LambdaInitializedService& s) {
    s.factor = 3.14;
    s.mode = "active";
});

// 4. 命名类实例注册
AUTO_REGISTER_CLASS_INSTANCE(DatabaseConnection, PrimaryDB);
#if 1
// 5. 带成员函数初始化的命名实例
AUTO_REGISTER_CLASS_INSTANCE_WITH_INITFUNC(DatabaseConnection, SecondaryDB, connect);

// 6. 带 Lambda 初始化的命名实例
AUTO_REGISTER_CLASS_INSTANCE_WITH_INIT(DatabaseConnection, ReadReplica, [](DatabaseConnection& db) {
    db.connectionString = "jdbc:mysql://replica.host/db";
    db.isConnected = true;
});

// 7. 类创建器注册 (Lambda)
AUTO_REGISTER_CLASS_CREATOR(ComplexObject, []() {
    return std::make_shared<ComplexObject>(100, "CreatorLambda");
});

// 8. 类创建器带成员函数初始化
AUTO_REGISTER_CLASS_CREATOR_WITH_INITFUNC(ComplexObject, []() {
    return std::make_shared<ComplexObject>(200, "CreatorInitFunc");
}, describe);

// 9. 类创建器带 Lambda 初始化
AUTO_REGISTER_CLASS_CREATOR_WITH_INIT(ComplexObject, []() {
    return std::make_shared<ComplexObject>(300, "CreatorLambdaInit");
}, [](ComplexObject& obj) {
    obj.id += 1000; // Modify after creation
});

// 10. 命名实例的类创建器
AUTO_REGISTER_CLASS_CREATOR_INSTANCE(ComplexObject, InstanceAlpha, []() {
    return std::make_shared<ComplexObject>(400, "InstCreatorAlpha");
});

// 11. 命名实例的类创建器带成员函数初始化
AUTO_REGISTER_CLASS_CREATOR_INSTANCE_WITH_INITFUNC(ComplexObject, InstanceBeta, []() {
    return std::make_shared<ComplexObject>(500, "InstCreatorBeta");
}, describe);

// 12. 命名实例的类创建器带 Lambda 初始化
AUTO_REGISTER_CLASS_CREATOR_INSTANCE_WITH_INIT(ComplexObject, InstanceGamma, []() {
    return std::make_shared<ComplexObject>(600, "InstCreatorGamma");
}, [](ComplexObject& obj) {
    obj.type = "ModifiedByLambda";
});

#endif
// --- 主函数 ---
int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "   AutoRegister Framework Demo         " << std::endl;
    std::cout << "========================================" << std::endl;

    // 打印注册表信息
    AutoRegister::instance().printRegistry();
    std::cout << std::endl;

    // 执行所有初始化 (会触发带初始化的注册的 init 函数)
    std::cout << "--- Executing all initializers ---" << std::endl;
    AutoRegister::instance().executeAllInits();
    std::cout << std::endl;

    // 获取并使用实例
    std::cout << "--- Getting and using instances ---" << std::endl;

    // 1. SimpleService
    auto simple = AutoRegister::instance().getInstance<SimpleService>("SimpleService");
    if (simple) simple->sayHello();

    // 2. ConfiguredService (already initialized)
    auto configured = AutoRegister::instance().getInstance<ConfiguredService>("ConfiguredService");
    if (configured) configured->show();

    // 3. LambdaInitializedService (already initialized)
    auto lambdaInit = AutoRegister::instance().getInstance<LambdaInitializedService>("LambdaInitializedService");
    if (lambdaInit) lambdaInit->setup();

    // 4. Named DB Instances
    auto primaryDb = AutoRegister::instance().getInstanceByName<DatabaseConnection>("DatabaseConnection", "PrimaryDB");
    if (primaryDb) {
        std::cout << "[Main] PrimaryDB created. Connecting now..." << std::endl;
        primaryDb->connect();
    }

    auto secondaryDb = AutoRegister::instance().getInstanceByName<DatabaseConnection>("DatabaseConnection", "SecondaryDB");
    if (secondaryDb) {
        std::cout << "[Main] SecondaryDB state: " << (secondaryDb->isConnected ? "Connected" : "Disconnected") << std::endl;
    }

    auto replicaDb = AutoRegister::instance().getInstanceByName<DatabaseConnection>("DatabaseConnection", "ReadReplica");
    if (replicaDb) {
        std::cout << "[Main] ReplicaDB connected to: " << replicaDb->connectionString << std::endl;
    }

    // 5. ComplexObjects from Creators
    auto creatorObj = AutoRegister::instance().getInstance<ComplexObject>("ComplexObject");
    if (creatorObj) creatorObj->describe();

    auto creatorInitFuncObj = AutoRegister::instance().getInstance<ComplexObject>("ComplexObject");
    if (creatorInitFuncObj) creatorInitFuncObj->describe();

    auto creatorLambdaInitObj = AutoRegister::instance().getInstance<ComplexObject>("ComplexObject");
    if (creatorLambdaInitObj) creatorLambdaInitObj->describe();

    // 6. Named ComplexObject Instances from Creators
    auto alpha = AutoRegister::instance().getInstanceByName<ComplexObject>("ComplexObject", "InstanceAlpha");
    if (alpha) alpha->describe();

    auto beta = AutoRegister::instance().getInstanceByName<ComplexObject>("ComplexObject", "InstanceBeta");
    if (beta) beta->describe();

    auto gamma = AutoRegister::instance().getInstanceByName<ComplexObject>("ComplexObject", "InstanceGamma");
    if (gamma) gamma->describe();


    std::cout << std::endl;
    std::cout << "--- Final Instance Count: " << AutoRegister::instance().getInstanceCount() << " ---" << std::endl;
    AutoRegister::instance().printInstances();

    return 0;
}
