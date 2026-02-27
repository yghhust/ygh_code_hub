#include "auto_register.h"
#include <iostream>
#include <string>

// ==================== 测试类定义 ====================

struct SimpleClass {
    int value = 0;
    ~SimpleClass() { std::cout << "[Dtor] SimpleClass\n"; }
};

struct InitClass {
    int value = 0;
    void init() { value = 10; }
    ~InitClass() { std::cout << "[Dtor] InitClass\n"; }
};

struct MemberInitClass {
    int value = 0;
    void setup() { value = 20; }
    ~MemberInitClass() { std::cout << "[Dtor] MemberInitClass\n"; }
};

struct CustomCreatorClass {
    int value = 0;
    explicit CustomCreatorClass(int v) : value(v) {}
    ~CustomCreatorClass() { std::cout << "[Dtor] CustomCreatorClass\n"; }
};

struct YetAnotherCustomCreatorClass {
    int value = 0;
    explicit YetAnotherCustomCreatorClass(int v) : value(v) {}
    ~YetAnotherCustomCreatorClass() { std::cout << "[Dtor] YetAnotherCustomCreatorClass\n"; }
};

struct FinalCustomCreatorClass {
    int value = 0;
    explicit FinalCustomCreatorClass(int v) : value(v) {}
    ~FinalCustomCreatorClass() { std::cout << "[Dtor] FinalCustomCreatorClass\n"; }
};

struct NamedClass {
    std::string name;
    int value = 0;
    void init() { value = 30; }
    ~NamedClass() { std::cout << "[Dtor] NamedClass\n"; }
};

struct NamedMemberInitClass {
    std::string name;
    int value = 0;
    void setup() { value = 40; }
    ~NamedMemberInitClass() { std::cout << "[Dtor] NamedMemberInitClass\n"; }
};

struct CustomCreatorMemberInitClass {
    int value = 0;
    void setup() { value = 50; }
    ~CustomCreatorMemberInitClass() { std::cout << "[Dtor] CustomCreatorMemberInitClass\n"; }
};

struct AnotherCustomCreatorMemberInitClass {
    int value = 0;
    AnotherCustomCreatorMemberInitClass() : value(0) {}
    void setup() { value = 60; }
    ~AnotherCustomCreatorMemberInitClass() { std::cout << "[Dtor] AnotherCustomCreatorMemberInitClass\n"; }
};

struct BrandNewCustomCreatorClass {
    int value = 0;
    BrandNewCustomCreatorClass() : value(0) {}
    explicit BrandNewCustomCreatorClass(int v) : value(v) {}
    ~BrandNewCustomCreatorClass() { std::cout << "[Dtor] BrandNewCustomCreatorClass\n"; }
};

struct BrandNewCustomCreatorMemberInitClass {
    int value = 0;
    BrandNewCustomCreatorMemberInitClass() : value(0) {}
    void setup() { value = 70; }
    ~BrandNewCustomCreatorMemberInitClass() { std::cout << "[Dtor] BrandNewCustomCreatorMemberInitClass\n"; }
};

// ==================== 使用所有宏进行注册（每类仅一次） ====================

AUTO_REG_CLASS(SimpleClass);
AUTO_REG_CLASS_PRI(InitClass, 3)
AUTO_REG_CLASS_INIT(MemberInitClass, [](MemberInitClass& o) { o.setup(); })
AUTO_REG_CLASS_INIT_PRI(BrandNewCustomCreatorClass, [](BrandNewCustomCreatorClass& o) { o.value = 999; }, 2)
AUTO_REG_NAMED(NamedClass, Alpha)
AUTO_REG_NAMED_PRI(NamedClass, Beta, 1)
AUTO_REG_NAMED_INIT(NamedClass, Gamma, [](NamedClass& o) { o.init(); })
AUTO_REG_NAMED_INIT_PRI(NamedClass, Delta, [](NamedClass& o) { o.init(); }, 4)
AUTO_REG_CREATOR(CustomCreatorClass, []() { return std::make_shared<CustomCreatorClass>(100); })
AUTO_REG_CREATOR_PRI(BrandNewCustomCreatorClass, []() { return std::make_shared<BrandNewCustomCreatorClass>(200); }, 6)
AUTO_REG_CREATOR_INIT(YetAnotherCustomCreatorClass, []() { return std::make_shared<YetAnotherCustomCreatorClass>(300); }, [](YetAnotherCustomCreatorClass& o) { o.value += 1; })
AUTO_REG_CREATOR_INIT_PRI(FinalCustomCreatorClass, []() { return std::make_shared<FinalCustomCreatorClass>(400); }, [](FinalCustomCreatorClass& o) { o.value += 2; }, 7)
AUTO_REG_CREATOR_NAMED(CustomCreatorClass, Epsilon, []() { return std::make_shared<CustomCreatorClass>(500); })
AUTO_REG_CREATOR_NAMED_PRI(CustomCreatorClass, Zeta, []() { return std::make_shared<CustomCreatorClass>(600); }, 8)
AUTO_REG_CREATOR_NAMED_INIT(CustomCreatorClass, Eta, []() { return std::make_shared<CustomCreatorClass>(700); }, [](CustomCreatorClass& o) { o.value += 3; })
AUTO_REG_CREATOR_NAMED_INIT_PRI(CustomCreatorClass, Theta, []() { return std::make_shared<CustomCreatorClass>(800); }, [](CustomCreatorClass& o) { o.value += 4; }, 9)
AUTO_REG_CLASS_INITFUNC(MemberInitClass, setup)
AUTO_REG_CLASS_INITFUNC_PRI(BrandNewCustomCreatorMemberInitClass, setup, 5)
AUTO_REG_NAMED_INITFUNC(NamedMemberInitClass, Omega, setup)
AUTO_REG_NAMED_INITFUNC_PRI(NamedMemberInitClass, Xi, setup, 3)
AUTO_REG_CREATOR_INITFUNC(CustomCreatorMemberInitClass, []() { return std::make_shared<CustomCreatorMemberInitClass>(); }, setup)
AUTO_REG_CREATOR_INITFUNC_PRI(BrandNewCustomCreatorMemberInitClass, []() { return std::make_shared<BrandNewCustomCreatorMemberInitClass>(); }, setup, 4)
AUTO_REG_CREATOR_NAMED_INITFUNC(CustomCreatorMemberInitClass, Nu, []() { return std::make_shared<CustomCreatorMemberInitClass>(); }, setup)
AUTO_REG_CREATOR_NAMED_INITFUNC_PRI(BrandNewCustomCreatorMemberInitClass, Mu, []() { return std::make_shared<BrandNewCustomCreatorMemberInitClass>(); }, setup, 2)

// ==================== 主函数测试 ====================
int example2() {
    std::cout << "=== 开始执行初始化 ===\n";
		AutoRegister::instance().executePriorInits(10);
    
		std::cout << "\n=== 获取单例实例测试 ===\n";
    auto simple = AutoRegister::instance().getInstance<SimpleClass>();
    std::cout << "SimpleClass value: " << (simple ? std::to_string(simple->value) : "null") << "\n";

    auto initCls = AutoRegister::instance().getInstance<InitClass>();
    std::cout << "InitClass value: " << (initCls ? std::to_string(initCls->value) : "null") << "\n";

    auto memberInit = AutoRegister::instance().getInstance<MemberInitClass>();
    std::cout << "MemberInitClass value: " << (memberInit ? std::to_string(memberInit->value) : "null") << "\n";

    auto customCreator = AutoRegister::instance().getInstance<CustomCreatorClass>();
    std::cout << "CustomCreatorClass value: " << (customCreator ? std::to_string(customCreator->value) : "null") << "\n";

    auto yetAnotherCustomCreator = AutoRegister::instance().getInstance<YetAnotherCustomCreatorClass>();
    std::cout << "YetAnotherCustomCreatorClass value: " << (yetAnotherCustomCreator ? std::to_string(yetAnotherCustomCreator->value) : "null") << "\n";

    auto finalCustomCreator = AutoRegister::instance().getInstance<FinalCustomCreatorClass>();
    std::cout << "FinalCustomCreatorClass value: " << (finalCustomCreator ? std::to_string(finalCustomCreator->value) : "null") << "\n";

    auto brandNewCustomCreator = AutoRegister::instance().getInstance<BrandNewCustomCreatorClass>();
    std::cout << "BrandNewCustomCreatorClass value: " << (brandNewCustomCreator ? std::to_string(brandNewCustomCreator->value) : "null") << "\n";

    auto anotherCustomCreatorMemberInit = AutoRegister::instance().getInstance<AnotherCustomCreatorMemberInitClass>();
    std::cout << "AnotherCustomCreatorMemberInitClass value: " << (anotherCustomCreatorMemberInit ? std::to_string(anotherCustomCreatorMemberInit->value) : "null") << "\n";

    auto brandNewCustomCreatorMemberInit = AutoRegister::instance().getInstance<BrandNewCustomCreatorMemberInitClass>();
    std::cout << "BrandNewCustomCreatorMemberInitClass value: " << (brandNewCustomCreatorMemberInit ? std::to_string(brandNewCustomCreatorMemberInit->value) : "null") << "\n";

    std::cout << "\n=== 获取命名实例测试（改用 getInstance<T>(name)） ===\n";
    auto namedAlpha = AutoRegister::instance().getInstance<NamedClass>("Alpha");
    std::cout << "NamedClass Alpha value: " << (namedAlpha ? std::to_string(namedAlpha->value) : "null") << "\n";

    auto namedGamma = AutoRegister::instance().getInstance<NamedClass>("Gamma");
    std::cout << "NamedClass Gamma value: " << (namedGamma ? std::to_string(namedGamma->value) : "null") << "\n";

    auto customEpsilon = AutoRegister::instance().getInstance<CustomCreatorClass>("Epsilon");
    std::cout << "CustomCreatorClass Epsilon value: " << (customEpsilon ? std::to_string(customEpsilon->value) : "null") << "\n";

    auto namedOmega = AutoRegister::instance().getInstance<NamedMemberInitClass>("Omega");
    std::cout << "NamedMemberInitClass Omega value: " << (namedOmega ? std::to_string(namedOmega->value) : "null") << "\n";

    auto customNu = AutoRegister::instance().getInstance<CustomCreatorMemberInitClass>("Nu");
    std::cout << "CustomCreatorMemberInitClass Nu value: " << (customNu ? std::to_string(customNu->value) : "null") << "\n";
    
		std::cout << "\n=== 所有宏测试完成 ===\n";
    return 0;
}

