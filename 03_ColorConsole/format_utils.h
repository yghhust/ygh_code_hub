/*
* @file        format_utils.h
* @brief       类 C++20 std::format 的字符串格式化工具
*
* @author      yuguohua <ghy_hust@qq.com>
* @date        2026-02-23
* @copyright Copyright (c) 2026
*
* @version     1.5
* @par Revision History:
* - V1.0 2026-01-28  yuguohua: initial
* - V1.1 2026-01-29  yuguohua: 修复 {} 应取前面未显式占用的参数
* - V1.2 2026-02-22  yuguohua: 用状态机重构，修复 {  } 崩溃，简化逻辑
* - V1.3 2026-02-22  yuguohua: 修复 {:.1f} 等格式说明符占位符未被计数的问题
* - V1.4 2026-02-23  yuguohua: 引入正则表达式重构
* - V1.5 2026-02-23  yuguohua: 修复转义花括号 {{ 和 }} 被误识别为占位符的问题
*
* @note
* 1. 支持空占位符 {} 及带空格的 {  }
* 2. 支持索引占位符 {0}, {1}, ...
* 3. 支持格式说明符 {:.2f}, {:08x}, {:<10} 等
* 4. 支持转义花括号 {{ 和 }}（中间可有空白）
* 5. 支持混合使用 {} 和 {n}，{} 自动跳过已显式占用的索引
* 6. 兼容 GCC 的 std::regex（不使用高级断言）
* 7. 状态管理简化，无多余函数参数
* 8. 正确识别转义花括号，不将其误认为占位符
*/

#pragma once
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
#include <set>
#include <regex>

class Formatter {
public:
    template<typename... Args>
    static std::string format(const std::string& fmt, const Args&... args) {
        std::ostringstream oss;
        try {
            std::vector<Value> values = {Value(args)...};
            size_t num_args = values.size();

            // 第一遍：正则提取所有占位符（过滤转义的）
            std::vector<PlaceholderInfo> placeholders = parse_placeholders(fmt);

            if (placeholders.size() > num_args) {
                throw std::runtime_error("Not enough arguments for format string. Need " +
                                        std::to_string(placeholders.size()) + " but have " +
                                        std::to_string(num_args) + ".");
            }

            // 第二遍：正则驱动格式化输出
            format_impl(oss, fmt, values, placeholders);
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

    struct PlaceholderInfo {
        size_t position;
        size_t length;
        std::string content;
        enum class Type { EMPTY, INDEXED, FORMAT_ONLY } type;
        size_t index;
        std::string spec;
    };

    // 正则提取所有占位符，并过滤被转义的情况
    static std::vector<PlaceholderInfo> parse_placeholders(const std::string& fmt) {
        std::vector<PlaceholderInfo> result;

        // 第一步：找出所有转义花括号的位置（{{ 和 }}）
        std::set<size_t> escaped_positions;
        std::regex escape_re(R"(\{\{|\}\})");
        auto esc_begin = std::sregex_iterator(fmt.begin(), fmt.end(), escape_re);
        auto esc_end = std::sregex_iterator();
        for (auto it = esc_begin; it != esc_end; ++it) {
            escaped_positions.insert(it->position(0));
            escaped_positions.insert(it->position(0) + 1);
        }

        // 第二步：提取普通占位符 {...}
        std::regex simple_re(R"(\{[^{}]*\})");
        auto begin = std::sregex_iterator(fmt.begin(), fmt.end(), simple_re);
        auto end = std::sregex_iterator();

        for (auto it = begin; it != end; ++it) {
            const auto& match = *it;
            size_t pos = match.position(0);
            size_t len = match.length(0);

            // 检查占位符范围内是否有转义位置
            bool is_escaped = false;
            for (size_t p = pos; p < pos + len; ++p) {
                if (escaped_positions.count(p)) {
                    is_escaped = true;
                    break;
                }
            }

            if (is_escaped) {
                continue;
            }

            // 手动检查是否被反斜杠转义（前面有奇数个 \）
            size_t backslash_count = 0;
            size_t check_pos = pos;
            while (check_pos > 0 && fmt[check_pos - 1] == '\\') {
                backslash_count++;
                check_pos--;
            }

            if (backslash_count % 2 == 1) {
                continue;
            }

            PlaceholderInfo info;
            info.position = pos;
            info.length = len;
            info.content = match[0].str().substr(1, len - 2); // 去掉 {}
            parse_placeholder_content(info);
            result.push_back(info);
        }
        return result;
    }

    static void parse_placeholder_content(PlaceholderInfo& info) {
        auto is_ws = [](char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; };

        size_t start = 0, end = info.content.size();
        while (start < end && is_ws(info.content[start])) ++start;
        while (end > start && is_ws(info.content[end - 1])) --end;
        std::string trimmed = (start < end) ? info.content.substr(start, end - start) : "";

        if (trimmed.empty()) {
            info.type = PlaceholderInfo::Type::EMPTY;
        } else if (trimmed[0] == ':') {
            info.type = PlaceholderInfo::Type::FORMAT_ONLY;
            info.spec = trimmed.substr(1);
        } else {
            size_t num_start = (trimmed[0] == '+' || trimmed[0] == '-') ? 1 : 0;
            if (num_start < trimmed.size() && std::isdigit(static_cast<unsigned char>(trimmed[num_start]))) {
                size_t num_end = num_start;
                while (num_end < trimmed.size() && std::isdigit(static_cast<unsigned char>(trimmed[num_end]))) ++num_end;
                info.index = std::stoul(trimmed.substr(num_start, num_end - num_start));
                info.type = PlaceholderInfo::Type::INDEXED;
            } else {
                throw std::runtime_error("Named arguments are not supported. Invalid placeholder at position " +
                                        std::to_string(info.position) + ": {" + info.content + "}");
            }
        }
    }

    // 正则驱动的格式化输出
    static void format_impl(std::ostringstream& oss, const std::string& fmt,
                        const std::vector<Value>& values,
                        const std::vector<PlaceholderInfo>& placeholders) {
        // 状态在内部定义
        std::set<size_t> used_indices;
        size_t auto_idx = 0;

        // 预扫描：把所有显式索引加入 used_indices
        for (const auto& ph : placeholders) {
            if (ph.type == PlaceholderInfo::Type::INDEXED) {
                used_indices.insert(ph.index);
            }
        }

        // 正则匹配：转义的 {{ 或 }}，或普通占位符
        // 注意：{{ 和 }} 必须连续，中间不能有空格
        std::regex token_re(R"(\{\{|\}\}|(\{[^{}]*\}))");
        auto begin = std::sregex_iterator(fmt.begin(), fmt.end(), token_re);
        auto end = std::sregex_iterator();

        size_t ph_idx = 0;
        size_t last_pos = 0;

        for (auto it = begin; it != end; ++it) {
            const auto& match = *it;
            size_t match_pos = match.position(0);
            size_t match_len = match.length(0);
            std::string token = match[0].str();

            // 输出匹配位置之前的普通字符
            if (match_pos > last_pos) {
                oss << fmt.substr(last_pos, match_pos - last_pos);
            }

            // 处理 token
            if (token == "{{") {
                // 转义的 {{，输出单个 {
                oss << '{';
            } else if (token == "}}") {
                // 转义的 }}，输出单个 }
                oss << '}';
            } else {
                // 普通占位符
                if (ph_idx >= placeholders.size()) {
                    throw std::runtime_error("Unexpected '{' at position " + std::to_string(match_pos));
                }
                const PlaceholderInfo& ph = placeholders[ph_idx];
                if (ph.position != match_pos) {
                    throw std::runtime_error("Mismatched placeholder at position " + std::to_string(match_pos));
                }
                process_placeholder(oss, ph, values, used_indices, auto_idx);
                ph_idx++;
            }

            last_pos = match_pos + match_len;
        }

        // 输出最后一段普通字符
        if (last_pos < fmt.size()) {
            oss << fmt.substr(last_pos);
        }

        if (ph_idx != placeholders.size()) {
            throw std::runtime_error("Not all placeholders were processed.");
        }
    }

    static void process_placeholder(std::ostringstream& oss, const PlaceholderInfo& ph,
                                    const std::vector<Value>& values,
                                    std::set<size_t>& used_indices, size_t& auto_idx) {
        switch (ph.type) {
            case PlaceholderInfo::Type::EMPTY: {
                // 跳过所有已被显式索引占用的位置
                while (auto_idx < values.size() && used_indices.count(auto_idx)) {
                    ++auto_idx;
                }
                if (auto_idx >= values.size()) {
                    throw std::runtime_error("Missing argument for {} at position " + std::to_string(ph.position));
                }
                apply_with_format(oss, values[auto_idx], "");
                used_indices.insert(auto_idx);
                ++auto_idx;
                break;
            }
            case PlaceholderInfo::Type::INDEXED: {
                if (ph.index >= values.size()) {
                    oss << '{' << ph.content << '}';
                    return;
                }
                // 显式索引不检查 used_indices（允许多次使用）
                apply_with_format(oss, values[ph.index], "");
                // 但仍记录到 used_indices，防止自动分配重复
                used_indices.insert(ph.index);
                break;
            }
            case PlaceholderInfo::Type::FORMAT_ONLY: {
                // 跳过所有已被显式索引占用的位置
                while (auto_idx < values.size() && used_indices.count(auto_idx)) {
                    ++auto_idx;
                }
                if (auto_idx >= values.size()) {
                    throw std::runtime_error("Missing argument for {:...} at position " + std::to_string(ph.position));
                }
                apply_with_format(oss, values[auto_idx], ph.spec);
                used_indices.insert(auto_idx);
                ++auto_idx;
                break;
            }
        }
    }

    static void apply_with_format(std::ostringstream& oss, const Value& value, const std::string& spec) {
        if (spec.empty()) {
            oss << Value::get_string(value);
            return;
        }

        char fill = ' ';
        char align = '\0';
        int width = 0;
        int precision = -1;
        char type = '\0';
        bool zero_pad = false;
        size_t i = 0;

        if (i < spec.size() && spec[i] == '0') {
            zero_pad = true;
            ++i;
        }
        if (i < spec.size() && (spec[i] == '<' || spec[i] == '>' || spec[i] == '^')) {
            align = spec[i++];
        }
        while (i < spec.size() && std::isdigit(static_cast<unsigned char>(spec[i]))) {
            width = width * 10 + (spec[i++] - '0');
        }
        if (i < spec.size() && spec[i] == '.') {
            ++i;
            precision = 0;
            while (i < spec.size() && std::isdigit(static_cast<unsigned char>(spec[i]))) {
                precision = precision * 10 + (spec[i++] - '0');
            }
        }
        if (i < spec.size()) {
            type = spec[i];
        }

        if (zero_pad && !align) {
            align = '>';
        }
        if (align == '<') {
            oss << std::left;
        } else if (align == '>') {
            oss << std::right;
        } else if (align == '^') {
            oss << std::internal;
        }

        if (width > 0) {
            oss << std::setw(width);
        }
        if (zero_pad) {
            oss << std::setfill('0');
        }
        if (precision >= 0) {
            oss << std::setprecision(precision);
        }

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
