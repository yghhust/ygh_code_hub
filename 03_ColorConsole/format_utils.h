/*
 * @file        format_utils.h
 * @brief       类 C++20 std::format 的字符串格式化工具
 *
 * @author      yuguohua <ghy_hust@qq.com>
 * @date        2026-02-23
 * @copyright Copyright (c) 2026
 *
 * @version     1.7
 * @par Revision History:
 * - V1.0 2026-01-28  yuguohua: initial
 * - V1.1 2026-01-29  yuguohua: 修复 {} 应取前面未显式占用的参数
 * - V1.2 2026-02-22  yuguohua: 用状态机重构，修复 {  } 崩溃，简化逻辑
 * - V1.3 2026-02-22  yuguohua: 修复 {:.1f} 等格式说明符占位符未被计数的问题
 * - V1.4 2026-02-23  yuguohua: 引入正则表达式重构
 * - V1.5 2026-02-23  yuguohua: 修复转义花括号 {{ 和 }} 被误识别为占位符的问题
 * - V1.6 2026-02-24  yuguohua: 支持参数不足时用空字符串填充，支持参数多余时忽略多余参数
 * - V1.7 2026-03-05  yuguohua: 支持非标准占位符（如 {key=value}），原样输出
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
 * 9. 支持占位符数量 > 参数数量（用空字符串填充）
 * 10. 支持占位符数量 < 参数数量（忽略多余参数）
 * 11. 支持非标准占位符（如 {key=value}），原样输出
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
#include <cmath>
#include <bitset>
#include <cstdlib>
#include <cstdio>

class Formatter {
public:
    template <typename... Args>
    static std::string format(const std::string& fmt, const Args&... args) {
        std::ostringstream oss;
        try {
            std::vector<Value> values = {Value(args)...};
            size_t num_args = values.size();

            // 第一遍：正则提取所有占位符（过滤转义的）
            std::vector<PlaceholderInfo> placeholders = parse_placeholders(fmt);

            // 第二遍：正则驱动格式化输出
            format_impl(oss, fmt, values, placeholders);
        } catch (const std::exception& e) {
            std::cerr << "[Formatter Error] fmt = " << fmt << ",err = " << e.what() << std::endl;
            throw;
        }
        return oss.str();
    }

private:
    struct Value {
        std::variant<int,
                     long,
                     long long,
                     unsigned,
                     unsigned long,
                     unsigned long long,
                     float,
                     double,
                     long double,
                     char,
                     bool,
                     std::string,
                     const char*,
                     std::string_view>
            data;

        template <typename T>
        Value(const T& v)
            : data(v) {}

        template <typename T>
        static T get_int(const Value& v) {
            return std::visit(
                [](auto&& arg) -> T {
                    using ArgType = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_integral_v<ArgType>) {
                        return static_cast<T>(arg);
                    } else {
                        throw std::runtime_error("Value is not an integer");
                    }
                },
                v.data);
        }

        template <typename T>
        static T get_float(const Value& v) {
            return std::visit(
                [](auto&& arg) -> T {
                    using ArgType = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_floating_point_v<ArgType>) {
                        return static_cast<T>(arg);
                    } else {
                        throw std::runtime_error("Value is not a floating point number");
                    }
                },
                v.data);
        }

        static std::string get_string(const Value& v) {
            return std::visit(
                [](auto&& arg) -> std::string {
                    using ArgType = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<ArgType, std::string>) {
                        return arg;
                    } else if constexpr (std::is_same_v<ArgType, const char*>) {
                        return arg;
                    } else if constexpr (std::is_same_v<ArgType, std::string_view>) {
                        return std::string(arg);
                    } else if constexpr (std::is_same_v<ArgType, char>) {
                        return std::string(1, arg);
                    } else if constexpr (std::is_same_v<ArgType, bool>) {
                        return arg ? "true" : "false";
                    } else {
                        std::ostringstream oss;
                        oss << arg;
                        return oss.str();
                    }
                },
                v.data);
        }
    };

    struct PlaceholderInfo {
        size_t position;
        size_t length;
        std::string content;
        enum class Type { EMPTY, INDEXED, FORMAT_ONLY, INVALID } type;
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
            info.content = match[0].str().substr(1, len - 2);  // 去掉 {}
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
		    return;
		}

		// 检查是否是格式说明符（以:开头）
		if (trimmed[0] == ':') {
		    info.type = PlaceholderInfo::Type::FORMAT_ONLY;
		    info.spec = trimmed.substr(1);
		    return;
		}

		// 检查是否是有效的无符号整数索引
		bool is_valid_index = true;
		for (char c : trimmed) {
		    if (!std::isdigit(static_cast<unsigned char>(c))) {
		        is_valid_index = false;
		        break;
		    }
		}

		if (is_valid_index) {
		    try {
		        info.index = std::stoul(trimmed);
		        info.type = PlaceholderInfo::Type::INDEXED;
		    } catch (...) {
		        info.type = PlaceholderInfo::Type::INVALID;
		    }
		} else {
		    // 非标准占位符 - 原样输出
		    info.type = PlaceholderInfo::Type::INVALID;
		}
	}
    
    // 正则驱动的格式化输出
    static void format_impl(std::ostringstream& oss,
                            const std::string& fmt,
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
                    // 没有对应的占位符信息，原样输出
                    oss << token;
                } else {
                    const PlaceholderInfo& ph = placeholders[ph_idx];
                    if (ph.position != match_pos) {
                        // 位置不匹配，原样输出
                        oss << token;
                    } else {
                        process_placeholder(oss, ph, values, used_indices, auto_idx);
                        ph_idx++;
                    }
                }
            }

            last_pos = match_pos + match_len;
        }

        // 输出最后一段普通字符
        if (last_pos < fmt.size()) {
            oss << fmt.substr(last_pos);
        }
    }

    static void process_placeholder(std::ostringstream& oss,
                                    const PlaceholderInfo& ph,
                                    const std::vector<Value>& values,
                                    std::set<size_t>& used_indices,
                                    size_t& auto_idx) {
        switch (ph.type) {
            case PlaceholderInfo::Type::EMPTY: {
                // 跳过所有已被显式索引占用的位置
                while (auto_idx < values.size() && used_indices.count(auto_idx)) {
                    ++auto_idx;
                }
                
                if (auto_idx < values.size()) {
                    // 有足够参数
                    apply_with_format(oss, values[auto_idx], "");
                    used_indices.insert(auto_idx);
                    ++auto_idx;
                } else {
                    // 参数不足，用空字符串填充
                    oss << "";
                    // 增加auto_idx以避免无限循环
                    ++auto_idx;
                }
                break;
            }
            case PlaceholderInfo::Type::INDEXED: {
                if (ph.index < values.size()) {
                    // 索引有效
                    apply_with_format(oss, values[ph.index], "");
                    used_indices.insert(ph.index);
                } else {
                    // 索引超出范围，用空字符串填充
                    oss << "";
                }
                break;
            }
            case PlaceholderInfo::Type::FORMAT_ONLY: {
                // 跳过所有已被显式索引占用的位置
                while (auto_idx < values.size() && used_indices.count(auto_idx)) {
                    ++auto_idx;
                }
                
                if (auto_idx < values.size()) {
                    // 有足够参数
                    apply_with_format(oss, values[auto_idx], ph.spec);
                    used_indices.insert(auto_idx);
                    ++auto_idx;
                } else {
                    // 参数不足，用空字符串填充
                    oss << "";
                    // 增加auto_idx以避免无限循环
                    ++auto_idx;
                }
                break;
            }
            case PlaceholderInfo::Type::INVALID: {
                // 非标准占位符，原样输出
                oss << '{' << ph.content << '}';
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

        // 预编译正则（静态，只编译一次）
        static const std::regex simple_regex(
            "([<^>]?)([0-9]+)?(\\.[0-9]+)?([bBdfxoXeEgG]?)$"
        );

        std::smatch match;

        // 1. 提取填充字符
        size_t i = 0;
        if (i < spec.size()) {
            char first = spec[i];
            // 0 是特例，允许作为填充字符（如 {0>8x}）
            if (first == '0') {
                fill = '0';
                ++i;
            }
            // 其他非数字、非 < ^ > . 的字符，作为填充字符
            else if (first != '<' && first != '^' && first != '>' && first != '.' && 
                     !std::isdigit(static_cast<unsigned char>(first))) {
                fill = first;
                ++i;
            }
        }

        // 2. 正则解析剩余部分 [align][width][.precision][type]
        std::string rest = spec.substr(i);
        if (std::regex_match(rest, match, simple_regex)) {
            if (match[1].matched) align = match[1].str()[0];
            if (match[2].matched) width = std::stoi(match[2].str());
            if (match[3].matched) precision = std::stoi(match[3].str().substr(1));
            if (match[4].matched) type = match[4].str()[0];
        }

        // 3. 兼容旧式 0 填充：{0>8x} 或 {08x}
        if (fill == '0' && !align && width > 0) {
            align = '>';
        }

        // 4. 设置对齐
        if (align == '<') oss << std::left;
        else if (align == '>') oss << std::right;
        else if (align == '^') oss << std::internal;
        else oss << std::right;  // 默认右对齐

        // 5. 设置宽度
        if (width > 0) oss << std::setw(width);

        // 6. 设置填充字符
        if (fill != ' ') oss << std::setfill(fill);

        // 7. 设置精度
        if (precision >= 0) oss << std::setprecision(precision);

        // 8. 根据类型输出
        try {
            switch (type) {
                case 'd': 
                    oss << std::dec << Value::get_int<long long>(value); 
                    break;
                case 'x': 
                    oss << std::hex << std::nouppercase << Value::get_int<unsigned long long>(value); 
                    break;
                case 'X': 
                    oss << std::hex << std::uppercase << Value::get_int<unsigned long long>(value); 
                    break;
                case 'o': 
                    oss << std::oct << Value::get_int<unsigned long long>(value); 
                    break;
                case 'f': 
                case 'F': 
                    oss << std::fixed << Value::get_float<long double>(value); 
                    break;
                case 'e': 
                    oss << std::scientific << Value::get_float<long double>(value); 
                    break;
                case 'g': 
                    oss << std::defaultfloat << Value::get_float<long double>(value); 
                    break;
                case 'b':
                case 'B': {
                    // 二进制格式
                    unsigned long long val = Value::get_int<unsigned long long>(value);
                    std::string bin = "";
                    if (val == 0) {
                        bin = "0";
                    } else {
                        while (val > 0) {
                            bin = (val & 1 ? "1" : "0") + bin;
                            val >>= 1;
                        }
                    }
                    
                    // 应用宽度和对齐
                    if (width > static_cast<int>(bin.size())) {
                        if (align == '<') {
                            bin += std::string(width - bin.size(), fill);
                        } else if (align == '^') {
                            size_t pad = width - bin.size();
                            size_t left_pad = pad / 2;
                            size_t right_pad = pad - left_pad;
                            bin = std::string(left_pad, fill) + bin + std::string(right_pad, fill);
                        } else { // '>' or default
                            bin = std::string(width - bin.size(), fill) + bin;
                        }
                    }
                    oss << bin;
                    break;
                }
                case 'c': {
                    // 字符格式
                    if (std::holds_alternative<char>(value.data)) {
                        oss << std::get<char>(value.data);
                    } else if (std::holds_alternative<int>(value.data)) {
                        oss << static_cast<char>(std::get<int>(value.data));
                    } else {
                        oss << Value::get_string(value);
                    }
                    break;
                }
                case 's': {
                    // 字符串格式
                    oss << Value::get_string(value);
                    break;
                }
                default:
                    // 未知类型，按字符串处理
                    oss << Value::get_string(value);
                    break;
            }
        } catch (const std::exception& e) {
            // 类型转换失败时，按字符串处理
            oss << Value::get_string(value);
        }

        // 9. 恢复默认格式（避免影响后续输出）
        oss << std::setfill(' ') << std::setw(0) << std::setprecision(6);
    }
};
