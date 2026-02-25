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
#include <limits>
#include <type_traits>
#include "format_utils.h"

// 前向声明
class ColorCout;

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
    // ========== 字符串带默认值版本 ==========
    static std::string getline(const std::string& prompt = "", const std::string_view& color = Color::RESET, const std::string& defaultVal = "") {
    	return getline_op(prompt, color).value_or(defaultVal);
    }
    
    // ========== 通用类型带默认值版本（数值等） ==========
    template<typename T>
    static T get(const std::string& prompt = "", const std::string_view& color = Color::RESET, const T& defaultVal = 0) {
        return get_op<T>(prompt, color).value_or(defaultVal);
    }
    
    // ========== 字符串版本 ==========
    static std::optional<std::string> getline_op(
        const std::string& prompt = "",
        const std::string_view& color = Color::RESET) {
        ColorGuard guard(color);  // RAII 颜色管理

        if (!prompt.empty()) {
            std::cout << prompt;
        }

        std::string value;
        if (std::getline(std::cin, value)) {
            return value;
        }
        
        // 读取失败，清除错误标志并忽略无效输入
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        
        return std::nullopt;
    }

    // ========== 通用类型版本（数值等） ==========
    template<typename T>
    static std::optional<T> get_op(
        const std::string& prompt = "",
        const std::string_view& color = Color::RESET) {
        ColorGuard guard(color);

        if (!prompt.empty()) {
            std::cout << prompt;
        }

        T value{};
        if (std::cin >> value) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
        // 读取失败，清除错误标志并忽略无效输入
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        
        return std::nullopt;
    }

private:
    // RAII 颜色守卫，确保退出时重置颜色
    class ColorGuard {
        std::string_view color_;
    public:
        explicit ColorGuard(std::string_view color) : color_(color) {
            std::cout << color_;
        }
        ~ColorGuard() {
            std::cout << Color::RESET;
        }
    };
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

public:    
    // 输出运算符
    template<typename T>
    ColorStream& operator<<(const T& value) {
        std::cout << value;
        return *this;
    }
  
    template<typename T>
    ColorStream& operator>>(T& value) {
        std::cin >> value;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return *this;
    }
    
    ColorStream& operator>>(std::string& value) {
        std::getline(std::cin, value);
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
inline ColorStream& redStream() {
    static ColorStream instance(Color::RED);
    return instance;
}
inline ColorStream& greenStream() { 
    static ColorStream instance(Color::GREEN);
    return instance;
}
inline ColorStream& yellowStream() { 
    static ColorStream instance(Color::YELLOW);
    return instance;
}
inline ColorStream& blueStream() { 
    static ColorStream instance(Color::BLUE);
    return instance;
}
inline ColorStream& magentaStream() { 
    static ColorStream instance(Color::MAGENTA);
    return instance;
}
inline ColorStream& cyanStream() { 
    static ColorStream instance(Color::CYAN);
    return instance;
}
inline ColorStream& whiteStream() { 
    static ColorStream instance(Color::WHITE);
    return instance;
}
inline ColorStream& boldredStream() { 
    static ColorStream instance(Color::BOLD_RED);
    return instance;
}
inline ColorStream& boldgreenStream() { 
    static ColorStream instance(Color::BOLD_GREEN);
    return instance;
}
inline ColorStream& boldyellowStream() { 
    static ColorStream instance(Color::BOLD_YELLOW);
    return instance;
}
inline ColorStream& boldblueStream() { 
    static ColorStream instance(Color::BOLD_BLUE);
    return instance;
}
inline ColorStream& errStream() { 
    static ColorStream instance(Color::ERROR);
    return instance;
}
inline ColorStream& warnStream() { 
    static ColorStream instance(Color::WARNING);
    return instance;
}
inline ColorStream& succStream() { 
    static ColorStream instance(Color::SUCCESS);
    return instance;
}
inline ColorStream& infoStream() { 
    static ColorStream instance(Color::INFO);
    return instance;
}

