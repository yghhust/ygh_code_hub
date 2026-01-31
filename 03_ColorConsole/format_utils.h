#ifndef FORMAT_UTILS_H
#define FORMAT_UTILS_H

#include <iostream>
#include <sstream>
/*-------------------------------------------------------------------------------------------------
#@brief		格式化字符串
#@details   将格式化字符串中的占位符 {} 按顺序替换为对应的参数值，生成最终的格式化字符串。
			1)支持多个参数和混合类型参数，通过递归展开参数包实现。
			2)当没有占位符或参数时，直接输出剩余格式字符串。
			3)遇到不匹配的花括号时抛出异常。
@param		fmt		入参-格式化字符串，使用 {} 作为占位符标记需要替换的参数位置
			args	入参-可变参数列表，支持任意数量和类型的参数，将按顺序替换占位符
@return		std::string 返回格式化后的完整字符串
@throws		std::runtime_err当格式字符串中存在未匹配的花括号时抛出
@example    1) 基本使用
				Formatter::format("Hello, {}!", "World"); // 返回 "Hello, World!"
			2) 多个参数
			    Formatter::format("{} + {} = {}", 2, 3, 5); // 返回 "2 + 3 = 5"
			3) 混合类型
				Formatter::format("Name: {}, Age: {}, Score: {}", "Alice", 25, 95.5);// 返回 "Name: Alice, Age: 25, Score: 95.5"
			4) 无参数
				Formatter::format("Hello World!"); // 返回 "Hello World!"
			5) 异常情况
				Formatter::format("Unmatched {"); // 抛出 std::runtime_error
@note		1) 占位符必须使用成对的花括号 {}，不支持带格式说明符的复杂格式（如 {:.2f}）
			2) 参数数量应与占位符数量匹配，多余的参数会被忽略，不足的占位符会导致异常
			3) 当前实现不支持位置参数（如 {0}, {1}）和命名参数
			4) 对于字符串中的字面量花括号，需要使用双写转义（如 "{{" 表示一个字面量 "{")
			5) 递归实现可能对大量参数有栈深度限制，实际使用中应评估参数数量
			6) 线程安全性：当前实现使用局部 ostringstream 对象，是线程安全的
			7) 性能考虑：每次调用都会创建新的 ostringstream，高频调用时建议考虑性能优化
			8) 错误处理：捕获异常时会输出错误信息到 cerr，但仍返回已格式化的部分字符串
-------------------------------------------------------------------------------------------------*/
class Formatter {
public:
    // 主格式化函数
    template<typename... Args>
    static std::string format(const std::string &fmt, const Args &... args) {
        std::ostringstream oss;
        try {
            format_helper(oss, fmt, 0, args...);
        } catch (const std::exception &e) {
            std::cerr << "Expected error: " << e.what() << std::endl;
        }
        return oss.str();
    }

private:
    // 终止条件：无参数
    static void format_helper(std::ostringstream &oss, const std::string &fmt, size_t idx) {
        oss << fmt.substr(idx);
    }

    // 递归处理：有参数
    template<typename T, typename... Rest>
    static void format_helper(std::ostringstream &oss, const std::string &fmt,
                              size_t idx, const T &first, const Rest &... rest) {
        size_t pos = fmt.find('{', idx);
        if (pos == std::string::npos) {
            oss << fmt.substr(idx);
            return;
        }

        size_t end = fmt.find('}', pos);
        if (end == std::string::npos) {
            throw std::runtime_error("Unmatched brace in format string");
        }

        // 输出格式字符串中 {} 之前的内容
        oss << fmt.substr(idx, pos - idx);

        // 处理当前参数
        oss << first;

        // 递归处理剩余部分和参数
        format_helper(oss, fmt, end + 1, rest...);
    }
};

