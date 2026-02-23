/*
 * @file        format_utils.h
 * @brief       类 C++20 std::format 的字符串格式化工具
 *
 * @author      yuguohua <ghy_hust@qq.com>
 * @date        2026-02-23
 * @copyright Copyright (c) 2026
 *
 * @version     1.0
 * @par Revision History:
 * - V1.0 2026-02-23  yuguohua: initial
 *
 * @note
 * 1. 支持空占位符 {}
 * 2. 支持索引占位符 {0}, {1}, ...
 * 3. 支持格式说明符 {:.2f}, {:08x}, {:<10} 等
 * 4. 支持转义花括号 {{ 和 }}
 */

#ifndef FORMAT_UTILS_H
#define FORMAT_UTILS_H

#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <type_traits>
#include <iomanip>
#include <cctype>
#include <utility>
#include <variant>
#include <vector>

class Formatter {
public:
    template<typename... Args>
    static std::string format(const std::string& fmt, const Args&... args) {
        std::ostringstream oss;
        try {
            std::vector<Value> values = {Value(args)...};
            size_t next_arg = 0;
            format_impl(oss, fmt, 0, next_arg, values);
        } catch (const std::exception& e) {
            std::cerr << "[Formatter Error] " << e.what() << std::endl;
            throw;
        }
        return oss.str();
    }

private:
    struct Value {
        std::variant<
            int, long, long long,
            unsigned, unsigned long, unsigned long long,
            float, double, long double,
            char, bool,
            std::string, const char*
        > data;

        template<typename T>
        Value(const T& v) : data(v) {}

        template<typename T>
        static T get_int(const Value& v) {
            return std::visit([](auto&& arg) -> T {
                using ArgType = std::decay_t<decltype(arg)>;
                if constexpr (std::is_integral_v<ArgType>) {
                    return static_cast<T>(arg);
                } else {
                    throw std::runtime_error("Value is not an integer");
                }
            }, v.data);
        }

        template<typename T>
        static T get_float(const Value& v) {
            return std::visit([](auto&& arg) -> T {
                using ArgType = std::decay_t<decltype(arg)>;
                if constexpr (std::is_floating_point_v<ArgType>) {
                    return static_cast<T>(arg);
                } else {
                    throw std::runtime_error("Value is not a floating point number");
                }
            }, v.data);
        }

        static std::string get_string(const Value& v) {
            return std::visit([](auto&& arg) -> std::string {
                using ArgType = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<ArgType, std::string>) {
                    return arg;
                } else if constexpr (std::is_same_v<ArgType, const char*>) {
                    return arg;
                } else if constexpr (std::is_same_v<ArgType, char>) {
                    return std::string(1, arg);
                } else if constexpr (std::is_same_v<ArgType, bool>) {
                    return arg ? "true" : "false";
                } else {
                    std::ostringstream oss;
                    oss << arg;
                    return oss.str();
                }
            }, v.data);
        }
    };

    static void format_impl(std::ostringstream& oss, const std::string& fmt,
                            size_t fmt_idx, size_t& next_arg,
                            const std::vector<Value>& values) {
        size_t i = fmt_idx;
        while (i < fmt.size()) {
            if (fmt[i] == '{') {
                if (i + 1 < fmt.size() && fmt[i + 1] == '{') {
                    oss << '{';
                    i += 2;
                    continue;
                }
                size_t j = fmt.find('}', i + 1);
                if (j == std::string::npos) {
                    throw std::runtime_error("Unmatched '{' at position " + std::to_string(i));
                }
                std::string placeholder = fmt.substr(i + 1, j - i - 1);
                process_placeholder(oss, placeholder, next_arg, values);
                i = j + 1;
            } else if (fmt[i] == '}') {
                if (i + 1 < fmt.size() && fmt[i + 1] == '}') {
                    oss << '}';
                    i += 2;
                    continue;
                }
                throw std::runtime_error("Unmatched '}' at position " + std::to_string(i));
            } else {
                oss << fmt[i];
                ++i;
            }
        }
    }

    static void process_placeholder(std::ostringstream& oss, const std::string& ph,
                                    size_t& next_arg,
                                    const std::vector<Value>& values) {
        if (ph.empty()) {
            if (next_arg >= values.size()) {
                throw std::runtime_error("Not enough arguments for format string. Missing argument for {}.");
            }
            oss << Value::get_string(values[next_arg]);
            next_arg++;
            return;
        }

        size_t colon_pos = ph.find(':');
        std::string arg_spec = (colon_pos != std::string::npos) ? ph.substr(colon_pos + 1) : "";
        std::string arg_key = (colon_pos != std::string::npos) ? ph.substr(0, colon_pos) : ph;

        if (arg_key.empty()) {
            if (next_arg >= values.size()) {
                throw std::runtime_error("Not enough arguments for format string. Missing argument for {:...}.");
            }
            apply_with_format(oss, values[next_arg], arg_spec);
            next_arg++;
            return;
        }

        if (std::isdigit(static_cast<unsigned char>(arg_key[0]))) {
            int pos = std::stoi(arg_key);
            if (pos < 0) {
                throw std::runtime_error("Position argument cannot be negative: " + arg_key);
            }
            if (static_cast<size_t>(pos) >= values.size()) {
                oss << '{' << arg_key;
                if (!arg_spec.empty()) {
                    oss << ':' << arg_spec;
                }
                oss << '}';
                return;
            }
            apply_with_format(oss, values[pos], arg_spec);
            return;
        } else {
            throw std::runtime_error("Named arguments are not supported. Use positional arguments like {}, {0}, {1}.");
        }
    }

    static void apply_with_format(std::ostringstream& oss, const Value& value, const std::string& spec) {
        if (spec.empty()) {
            oss << Value::get_string(value);
            return;
        }

        // 解析格式说明符
        char fill = ' ';
        char align = '\0';
        int width = 0;
        int precision = -1;  // -1 表示没有精度
        char type = '\0';
        bool zero_pad = false;

        size_t i = 0;
        
        // 解析对齐方式和填充字符
        if (i < spec.size() && spec[i] == '0') {
            zero_pad = true;
            ++i;
        }
        if (i < spec.size() && (spec[i] == '<' || spec[i] == '>' || spec[i] == '^')) {
            align = spec[i];
            ++i;
        }
        
        // 解析宽度
        while (i < spec.size() && std::isdigit(static_cast<unsigned char>(spec[i]))) {
            width = width * 10 + (spec[i] - '0');
            ++i;
        }
        
        // 解析精度（.2 部分）
        if (i < spec.size() && spec[i] == '.') {
            ++i;
            precision = 0;
            while (i < spec.size() && std::isdigit(static_cast<unsigned char>(spec[i]))) {
                precision = precision * 10 + (spec[i] - '0');
                ++i;
            }
        }
        
        // 解析类型（f, x, d 等）
        if (i < spec.size()) {
            type = spec[i];
        }

        // 设置格式
        if (zero_pad && !align) align = '>';
        if (align == '<') oss << std::left;
        else if (align == '>') oss << std::right;
        else if (align == '^') oss << std::internal;

        if (width > 0) oss << std::setw(width);
        if (zero_pad) oss << std::setfill('0');
        if (precision >= 0) oss << std::setprecision(precision);

        // 根据类型输出
        switch (type) {
            case 'd':
                oss << std::dec << Value::get_int<int>(value);
                break;
            case 'x':
                oss << std::hex << std::nouppercase << Value::get_int<unsigned int>(value);
                break;
            case 'X':
                oss << std::hex << std::uppercase << Value::get_int<unsigned int>(value);
                break;
            case 'o':
                oss << std::oct << Value::get_int<unsigned int>(value);
                break;
            case 'f':
                oss << std::fixed << Value::get_float<double>(value);
                break;
            case 'e':
                oss << std::scientific << Value::get_float<double>(value);
                break;
            case 'g':
                oss << std::defaultfloat << Value::get_float<double>(value);
                break;
            default:
                oss << Value::get_string(value);
                break;
        }

        oss << std::setfill(' ') << std::setw(0) << std::setprecision(6);
    }
};

#endif // FORMAT_UTILS_H
