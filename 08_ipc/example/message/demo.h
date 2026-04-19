// 自动生成的头文件，请勿手动修改
#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>
#include <variant>
#include <type_traits>

struct AddRequest {
    int64_t a;
    int64_t b;

    friend void to_json(nlohmann::json& j, const AddRequest& obj);
    friend void from_json(const nlohmann::json& j, AddRequest& obj);
};

struct SubRequest {
    int64_t a;
    int64_t b;

    friend void to_json(nlohmann::json& j, const SubRequest& obj);
    friend void from_json(const nlohmann::json& j, SubRequest& obj);
};

struct AddResponse {
    int64_t result;

    friend void to_json(nlohmann::json& j, const AddResponse& obj);
    friend void from_json(const nlohmann::json& j, AddResponse& obj);
};

struct SubResponse {
    int64_t result;

    friend void to_json(nlohmann::json& j, const SubResponse& obj);
    friend void from_json(const nlohmann::json& j, SubResponse& obj);
};

// ---------- Variant Type ----------
using calc_message = std::variant<
    AddRequest,
    AddResponse,
    SubRequest,
    SubResponse
>;

void to_json(nlohmann::json& j, const calc_message& cmd);
void from_json(const nlohmann::json& j, calc_message& cmd);
