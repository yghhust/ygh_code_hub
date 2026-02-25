#include <iostream>
#include "colorConsole.h"
#include <cmath>

int formatter() {
    std::cout << "=== Testing Formatter ===" << std::endl;
    
    // 基本使用
    std::cout << Formatter::format("Hello, {}!\n", "World");

    // 位置参数
    std::cout << Formatter::format("{1} + {0} = {2}\n", 2, 3, 5);
    std::cout << Formatter::format("{} + {0} = {2}\n", 12, 13, 25);

    // 格式说明符
    std::cout << Formatter::format("Pi ≈ {:.2f}\n", 3.1415926);
    std::cout << Formatter::format("Hex: {:0<8x}\n", 255);
    std::cout << Formatter::format("Name: {:<10}\n", "Bob");

    // 转义花括号
    std::cout << Formatter::format("{{Not a placeholder}} = {}\n", 42);

    // 混合类型
    std::cout << Formatter::format("Name: {}, Age: {1}, Score: {:.1f}", "Alice", 25, 95.5) << std::endl;

    std::cout << "=== Testing Formatter End===" << std::endl;
    
    return 0;
}


int colorCout() {
    std::cout << std::endl;
    std::cout << "=== Color Console Demo ===" << std::endl;
    std::cout << std::endl;

    // ========== 1. 基本颜色输出 ==========
    std::cout << "--- 1.Basic Color Output ---" << std::endl;
    ColorCout::println(Color::RED, "This is red text");
    ColorCout::println(Color::GREEN, "This is green text");
    ColorCout::println(Color::YELLOW, "This is yellow text");
    ColorCout::println(Color::BLUE, "This is blue text");
    ColorCout::println(Color::MAGENTA, "This is magenta text");
    ColorCout::println(Color::CYAN, "This is cyan text");
    ColorCout::println(Color::BOLD_WHITE, "This is bold white text");
    std::cout << std::endl;

    // ========== 2. 格式化输出 ==========
    std::cout << "--- 2.Formatted Output ---" << std::endl;
    ColorCout::println("Hello, World0!");
    ColorCout::println("Hello, {}!", "World");
    ColorCout::println(Color::RED, "Hello, world4!");
    ColorCout::println("Numbers: {1}, {0}, {}", 1, 2.5, 3);
    ColorCout::println("Hex: 0x{:08x}", 255);
    ColorCout::println("Float: {:.2f}", 3.14159);
    ColorCout::println("Scientific: {:.2e}", 123456.789);
    ColorCout::println("Binary-like: {:08b}", 5); 
    std::cout << std::endl;

    // ========== 3. 带颜色的格式化输出 ==========
    std::cout << "--- 3.Colored Formatted Output ---" << std::endl;
    ColorCout::println(Color::SUCCESS, "Operation successful! Result: {}", 42);
    ColorCout::println(Color::ERROR, "Error code: {}, Message: {}", 404, "Not Found");
    ColorCout::println(Color::WARNING, "Warning: Value {:.1f} is out of range", 99.9);
    ColorCout::println(Color::INFO, "Info: Processing file '{}'", "data.txt");
    std::cout << std::endl;

    // ========== 4. 流式输出 ==========
    std::cout << "--- 4.1 Stream Output ---" << std::endl;
    redStream() << "This is red stream output" << std::endl;
    greenStream() << "This is green stream output" << std::endl;
    blueStream() << "This is blue stream output" << std::endl;
    errStream() << "This is error stream output" << std::endl;
    succStream() << "This is success stream output" << std::endl;
    cyanStream() << "Multiple " << "words " << "in " << "one " << "line" << std::endl;
    std::cout << std::endl;
    std::cout << "--- 4.2 Stream Input ---" << std::endl;
    int a;
    redStream() << "Input int a:" >> a;
    std::string b;
    boldredStream() << "Input string b:" >> b;

    // ========== 5. 输入功能 ==========
    std::cout << "--- 5.Input Demo ---" << std::endl;
    std::string name = ColorCin::getline("Enter your name: ", Color::BOLD_CYAN);
    int age = ColorCin::get<int>("Enter your age: ", Color::BOLD_YELLOW);
    std::cout << std::endl;

    ColorCout::println(Color::GREEN, "Hello, {}! You are {} years old.", name, age);
    std::cout << std::endl;

    // ========== 6. 综合示例 ==========
    std::cout << "--- 6.Comprehensive Example ---" << std::endl;

    ColorCout::println(Color::BOLD_RED, R"(
    ╔══════════════════════════════════════╗
    ║     APPLICATION STARTED SUCCESSFULLY ║
    ╚══════════════════════════════════════╝
    )");

    ColorCout::println(Color::INFO, "System initialized in {:.3f} seconds", 1.234);
    ColorCout::println(Color::SUCCESS, "All services are running");
    ColorCout::println(Color::CYAN, "Version: {}.{}.{}", 1, 0, 0);
    std::cout << std::endl;

    std::cout << "=== Demo Complete ===" << std::endl;
    
    return 0;
}

int main() {
	formatter();
    colorCout();
    return 0;
}
