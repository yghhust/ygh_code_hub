//
// Created by ygh on 26-1-29.
//

#ifndef COLOR_CONSOLE_H
#define COLOR_CONSOLE_H

#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>

// ANSI 颜色代码定义
namespace Color {
    // 前景色
    constexpr const char* RESET = "\033[0m";
    constexpr const char* BLACK = "\033[30m";
    constexpr const char* RED = "\033[31m";
    constexpr const char* GREEN = "\033[32m";
    constexpr const char* YELLOW = "\033[33m";
    constexpr const char* BLUE = "\033[34m";
    constexpr const char* MAGENTA = "\033[35m";
    constexpr const char* CYAN = "\033[36m";
    constexpr const char* WHITE = "\033[37m";

    // 亮色前景
    constexpr const char* BOLD_BLACK = "\033[1;30m";
    constexpr const char* BOLD_RED = "\033[1;31m";
    constexpr const char* BOLD_GREEN = "\033[1;32m";
    constexpr const char* BOLD_YELLOW = "\033[1;33m";
    constexpr const char* BOLD_BLUE = "\033[1;34m";
    constexpr const char* BOLD_MAGENTA = "\033[1;35m";
    constexpr const char* BOLD_CYAN = "\033[1;36m";
    constexpr const char* BOLD_WHITE = "\033[1;37m";

    // 背景色
    constexpr const char* BG_RED = "\033[41m";
    constexpr const char* BG_GREEN = "\033[42m";
    constexpr const char* BG_YELLOW = "\033[43m";
    constexpr const char* BG_BLUE = "\033[44m";
    constexpr const char* BG_MAGENTA = "\033[45m";
    constexpr const char* BG_CYAN = "\033[46m";
    constexpr const char* BG_WHITE = "\033[47m";
}

class ColorCin {
public:
    // 静态方法，直接输出带颜色的文本
    static std::string getline(const std::string& color = Color::RESET) {
        //设置输出颜色
        std::cout << color;
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
    // 无颜色版本
    template<typename... Args>
    static void print(Args&&... args) {
        ((std::cout << args), ...);
    }

    // 有颜色版本
    template<typename... Args>
    static void print(const char* color, Args&&... args) {
        std::cout << color;
        ((std::cout << std::forward<Args>(args)), ...);
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
    template <typename T>
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
