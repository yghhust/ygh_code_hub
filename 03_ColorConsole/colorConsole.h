//
// Created by ygh on 26-1-29.
//

#ifndef COLOR_CONSOLE_H
#define COLOR_CONSOLE_H

#include <iostream>
#include <string>
#include "format_utils.h"
#include <string_view>

// ANSI 颜色代码定义
namespace Color {
    // 使用结构体包装字符串视图
    // 头文件中的推荐实现方式
    struct ColorCode {
        std::string_view value;

        friend std::ostream &operator<<(std::ostream &os, const ColorCode &c) {
            return os << c.value;
        }
    };

    // 基础控制
    constexpr ColorCode RESET{"\033[0m"};

    // 标准前景色
    constexpr ColorCode BLACK{"\033[30m"};
    constexpr ColorCode RED{"\033[31m"};
    constexpr ColorCode GREEN{"\033[32m"};
    constexpr ColorCode YELLOW{"\033[33m"};
    constexpr ColorCode BLUE{"\033[34m"};
    constexpr ColorCode MAGENTA{"\033[35m"};
    constexpr ColorCode CYAN{"\033[36m"};
    constexpr ColorCode WHITE{"\033[37m"};

    // 亮色前景
    constexpr ColorCode BOLD_BLACK{"\033[1;30m"};
    constexpr ColorCode BOLD_RED{"\033[1;31m"};
    constexpr ColorCode BOLD_GREEN{"\033[1;32m"};
    constexpr ColorCode BOLD_YELLOW{"\033[1;33m"};
    constexpr ColorCode BOLD_BLUE{"\033[1;34m"};
    constexpr ColorCode BOLD_MAGENTA{"\033[1;35m"};
    constexpr ColorCode BOLD_CYAN{"\033[1;36m"};
    constexpr ColorCode BOLD_WHITE{"\033[1;37m"};

    // 背景色
    constexpr ColorCode BG_RED{"\033[41m"};
    constexpr ColorCode BG_GREEN{"\033[42m"};
    constexpr ColorCode BG_YELLOW{"\033[43m"};
    constexpr ColorCode BG_BLUE{"\033[44m"};
    constexpr ColorCode BG_MAGENTA{"\033[45m"};
    constexpr ColorCode BG_CYAN{"\033[46m"};
    constexpr ColorCode BG_WHITE{"\033[47m"};
}

class ColorCin {
public:
    // 静态方法，直接输出带颜色的文本
    static std::string getline(const std::string &prompt = "", const Color::ColorCode &color = Color::RESET) {
        //设置输出颜色&输入提示
        std::cout << color << prompt;
        //输入
        std::string input;
        std::getline(std::cin, input);
        //恢复颜色设置
        std::cout << Color::RESET;

        return input;
    }
};

class ColorCout {
public:
    // 单参版本
    template<typename T>
    static void print(const T &value, const Color::ColorCode &color = Color::RESET) {
        std::cout << color << value << Color::RESET;
    }

    // 多参格式化版本
    template<typename... Args>
    static void print(const std::string &fmt, Args &&... args) {
        std::cout << Formatter::format(fmt, std::forward<Args>(args)...);
    }

    // 多参格式化版本(带颜色设置)
    template<typename... Args>
    static void print(const Color::ColorCode &color, const std::string &fmt, Args &&... args) {
        std::cout << color;
        std::cout << Formatter::format(fmt, std::forward<Args>(args)...);
        std::cout << Color::RESET;
    }

    // println 同理
    template<typename... Args>
    static void println(Args&&... args) {
        print(std::forward<Args>(args)...);
        std::cout << std::endl;
    }

    template<typename... Args>
    static void println(const char* color, Args&&... args) {
        print(color, std::forward<Args>(args)...);
        std::cout << std::endl;
    }

    // 链式输出
    template<typename T>
    ColorCout &operator<<(const T &value) {
        print(value);
        return *this;
    }
};


class ColorCout2 {
public:
    // 有颜色版本(不带换行)
    template<typename... Args>
    static void print(const char* color, Args&&... args) {
        std::cout << color;
        if constexpr (sizeof...(Args) > 0) {
            printWithSpaces(std::forward<Args>(args)...);
        }
        std::cout << Color::RESET;
    }

    // 无颜色版本(不带换行)
    template<typename... Args>
    static void print(Args&&... args) {
        print(Color::RESET, std::forward<Args>(args)...);
    }

    //有颜色版本(带换行)
    template<typename... Args>
    static void println(const char* color, Args&&... args) {
        print(color, std::forward<Args>(args)...);
        std::cout << std::endl;
    }

    // 无颜色版本(带换行)
    template<typename... Args>
    static void println(Args&&... args) {
        print(std::forward<Args>(args)...);
        std::cout << std::endl;
    }

private:
    template<typename First, typename... Rest>
    static void printWithSpaces(const First& first, const Rest&... rest) {
        std::cout << first;
        if constexpr (sizeof...(rest) > 0) {
            ((std::cout << " " << rest), ...);
        }
    }
};

class Formatter {
private:
    std::ostringstream oss;

public:
    // 各种格式化方法
    Formatter& width(int w) { oss << std::setw(w); return *this; }
    Formatter& fill(char c) { oss << std::setfill(c); return *this; }
    Formatter& precision(int p) { oss << std::setprecision(p); return *this; }
    Formatter& fixed() { oss << std::fixed; return *this; }
    Formatter& scientific() { oss << std::scientific; return *this; }
    Formatter& hex() { oss << std::hex; return *this; }
    Formatter& dec() { oss << std::dec; return *this; }
    Formatter& oct() { oss << std::oct; return *this; }

    // 模板方法添加任意值
    template<typename T>
    Formatter& add(const T& value) {
        oss << value;
        return *this;
    }

    // 转换为字符串
    std::string str() const { return oss.str(); }
    operator std::string() const { return oss.str(); }

    // 直接输出
    void print() const { std::cout << oss.str(); }
};
#endif
