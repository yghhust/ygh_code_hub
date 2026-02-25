/*
 * @file        color_console.h
 * @brief       ANSI 彩色控制台输入输出工具库
 *
 * @author      ygh <ghy_hust@qq.com>
 * @date        2026-02-23
 * @copyright   Copyright (c) 2026
 *
 * @version     2.0
 * @par Revision History:
 * - V1.0 2026-01-29  ygh: 初始版本
 * - V2.0 2026-02-23  ygh: 重构优化，统一接口，修复问题
 *
 * @note
 * 1. 支持多种颜色输出
 * 2. 支持格式化输出
 * 3. 支持链式调用
 * 4. 支持输入提示
 */

#pragma once

#include <iostream>
#include <string>
#include <string_view>
#include <sstream>
#include <iomanip>
#include <optional>
#include "format_utils.h"

// 前向声明
//class Formatter;

// ANSI 颜色代码定义
namespace Color {
    // 使用 inline 变量 (C++17)，避免多重定义
    inline constexpr std::string_view RESET{"\033[0m"};

    // 标准前景色
    inline constexpr std::string_view BLACK{"\033[30m"};
    inline constexpr std::string_view RED{"\033[31m"};
    inline constexpr std::string_view GREEN{"\033[32m"};
    inline constexpr std::string_view YELLOW{"\033[33m"};
    inline constexpr std::string_view BLUE{"\033[34m"};
    inline constexpr std::string_view MAGENTA{"\033[35m"};
    inline constexpr std::string_view CYAN{"\033[36m"};
    inline constexpr std::string_view WHITE{"\033[37m"};

    // 亮色前景
    inline constexpr std::string_view BOLD_BLACK{"\033[1;30m"};
    inline constexpr std::string_view BOLD_RED{"\033[1;31m"};
    inline constexpr std::string_view BOLD_GREEN{"\033[1;32m"};
    inline constexpr std::string_view BOLD_YELLOW{"\033[1;33m"};
    inline constexpr std::string_view BOLD_BLUE{"\033[1;34m"};
    inline constexpr std::string_view BOLD_MAGENTA{"\033[1;35m"};
    inline constexpr std::string_view BOLD_CYAN{"\033[1;36m"};
    inline constexpr std::string_view BOLD_WHITE{"\033[1;37m"};

    // 背景色
    inline constexpr std::string_view BG_RED{"\033[41m"};
    inline constexpr std::string_view BG_GREEN{"\033[42m"};
    inline constexpr std::string_view BG_YELLOW{"\033[43m"};
    inline constexpr std::string_view BG_BLUE{"\033[44m"};
    inline constexpr std::string_view BG_MAGENTA{"\033[45m"};
    inline constexpr std::string_view BG_CYAN{"\033[46m"};
    inline constexpr std::string_view BG_WHITE{"\033[47m"};

    // 常用组合
    inline constexpr std::string_view ERROR = BOLD_RED;
    inline constexpr std::string_view WARNING = BOLD_YELLOW;
    inline constexpr std::string_view SUCCESS = BOLD_GREEN;
    inline constexpr std::string_view INFO = BOLD_CYAN;
    inline constexpr std::string_view DEBUG = BOLD_WHITE;
}

// 输入类
class ColorCin {
public:
    // 带颜色提示的输入
    static std::string getline(
        const std::string& prompt = "",
        const std::string_view& color = Color::RESET
    ) {
        if (!prompt.empty()) {
            std::cout << color << prompt;
        }
        std::string input;
        std::getline(std::cin, input);
        std::cout << Color::RESET;
        return input;
    }

    // 带默认值的输入
    static std::string getline(
        const std::string& prompt,
        const std::string_view& color,
        const std::string& default_value
    ) {
        std::string input = getline(prompt, color);
        return input.empty() ? default_value : input;
    }

    // 获取整数输入
    static std::optional<int> getInt(
        const std::string& prompt = "",
        const std::string_view& color = Color::RESET
    ) {
        std::string input = getline(prompt, color);
        if (input.empty()) {
            return std::nullopt;
        }
        try {
            return std::stoi(input);
        } catch (...) {
            return std::nullopt;
        }
    }

    // 获取浮点数输入
    static std::optional<double> getDouble(
        const std::string& prompt = "",
        const std::string_view& color = Color::RESET
    ) {
        std::string input = getline(prompt, color);
        if (input.empty()) {
            return std::nullopt;
        }
        try {
            return std::stod(input);
        } catch (...) {
            return std::nullopt;
        }
    }
};

// 输出类 - 简单版本
class ColorCout {
public:
    // 格式化输出
    template<typename... Args>
    static void print(const std::string& fmt, const Args&... args) {
        std::cout << Formatter::format(fmt, args...) << Color::RESET;
    }

    // 带颜色的格式化输出
    template<typename... Args>
    static void print(const std::string_view& color, const std::string& fmt, const Args&... args) {
        std::cout << color << Formatter::format(fmt, args...) << Color::RESET;
    }

    // 格式化输出(换行)
    template<typename... Args>
    static void println(const std::string& fmt, Args&&... args) {
        print(fmt, std::forward<Args>(args)...);
        std::cout << '\n';
    }

    // 带颜色的格式化输出(换行)
    template<typename... Args>
    static void println(const std::string_view& color, const std::string& fmt, Args&&... args) {
        print(color, fmt, std::forward<Args>(args)...);
        std::cout << '\n';
    }
};

// 流式输出类
class ColorStream {
public:
    explicit ColorStream(const std::string_view& color = Color::RESET)
        : color_(color) {
        std::cout << color_;
    }

    ~ColorStream() {
        std::cout << Color::RESET;
    }

    // 禁止拷贝
    ColorStream(const ColorStream&) = delete;
    ColorStream& operator=(const ColorStream&) = delete;

    // 允许移动
    ColorStream(ColorStream&&) = default;
    ColorStream& operator=(ColorStream&&) = default;

    // 输出运算符
    template<typename T>
    ColorStream& operator<<(const T& value) {
        std::cout << value;
        return *this;
    }
  
    // 支持 std::endl 等特殊操纵符
    ColorStream& operator<<(std::ostream& (*manip)(std::ostream&)) {
        manip(std::cout);
        if (manip == static_cast<std::ostream& (*)(std::ostream&)>(std::endl)) {
            std::cout << color_;  // 换行后恢复颜色
        }
        return *this;
    }

private:
    std::string_view color_;
};

// 便捷的颜色流创建函数
inline ColorStream red() { return ColorStream(Color::RED); }
inline ColorStream green() { return ColorStream(Color::GREEN); }
inline ColorStream yellow() { return ColorStream(Color::YELLOW); }
inline ColorStream blue() { return ColorStream(Color::BLUE); }
inline ColorStream magenta() { return ColorStream(Color::MAGENTA); }
inline ColorStream cyan() { return ColorStream(Color::CYAN); }
inline ColorStream white() { return ColorStream(Color::WHITE); }
inline ColorStream bold_red() { return ColorStream(Color::BOLD_RED); }
inline ColorStream bold_green() { return ColorStream(Color::BOLD_GREEN); }
inline ColorStream bold_yellow() { return ColorStream(Color::BOLD_YELLOW); }
inline ColorStream bold_blue() { return ColorStream(Color::BOLD_BLUE); }
inline ColorStream error() { return ColorStream(Color::ERROR); }
inline ColorStream warning() { return ColorStream(Color::WARNING); }
inline ColorStream success() { return ColorStream(Color::SUCCESS); }
inline ColorStream info() { return ColorStream(Color::INFO); }

#if 1
// 链式格式化器
class ColorFormatter {
private:
    std::ostringstream oss;

public:
    explicit ColorFormatter(const std::string_view& c = Color::RESET){
        oss << c;
    }
    
    // 禁止拷贝
    ColorFormatter(const ColorFormatter&) = delete;
    ColorFormatter& operator=(const ColorFormatter&) = delete;

    // 允许移动
    ColorFormatter(ColorFormatter&&) = default;
    ColorFormatter& operator=(ColorFormatter&&) = default;

    // 设置颜色
    ColorFormatter& color(const std::string_view& c = Color::RESET) {
        oss << c;
        return *this;
    }

    // 宽度
    ColorFormatter& width(int w) {
        oss << std::setw(w);
        return *this;
    }

    // 填充字符
    ColorFormatter& fill(char c) {
        oss << std::setfill(c);
        return *this;
    }

    // 精度
    ColorFormatter& precision(int p) {
        oss << std::setprecision(p);
        return *this;
    }

    // 格式标志
    ColorFormatter& fixed() { oss << std::fixed; return *this; }
    ColorFormatter& scientific() { oss << std::scientific; return *this; }
    ColorFormatter& hex() { oss << std::hex; return *this; }
    ColorFormatter& dec() { oss << std::dec; return *this; }
    ColorFormatter& oct() { oss << std::oct; return *this; }
    ColorFormatter& left() { oss << std::left; return *this; }
    ColorFormatter& right() { oss << std::right; return *this; }
    ColorFormatter& internal() { oss << std::internal; return *this; }

    // 添加值
    template<typename T>
    ColorFormatter& add(const T& value) {
        oss << value;
        return *this;
    }

    // 格式化添加
    template<typename... Args>
    ColorFormatter& addf(const std::string& fmt, const Args&... args) {
        oss << Formatter::format(fmt, args...);
        return *this;
    }
    
    // 获取结果
    std::string str() const { return oss.str(); }
    operator std::string() const { return oss.str(); }

    // 输出
    void print() const { std::cout << oss.str() << Color::RESET; }
    void println() const { print(); std::cout << std::endl; }
};
#endif



