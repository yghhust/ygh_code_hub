#include "format_utils.h"
#include <iostream>

int main() {
    // 基本使用
    std::cout << Formatter::format("Hello, {}!\n", "World");

    // 位置参数
    std::cout << Formatter::format("{1} + {0} = {2}\n", 2, 3, 5);
    
		std::cout << Formatter::format("{} + {0} = {2}\n", 12, 13, 25);

    // 格式说明符
    std::cout << Formatter::format("Pi ≈ {:.2f}\n", 3.1415926);
    std::cout << Formatter::format("Hex: {:08x}\n", 255);
    std::cout << Formatter::format("Name: {:<10}\n", "Bob");

    // 转义花括号
    std::cout << Formatter::format("{{Not a placeholder}} = {}\n", 42);

    // 混合类型
    std::cout << Formatter::format("Name: {}, Age: {}, Score: {:.1f}", "Alice", 25, 95.5) << std::endl;

    return 0;
}
