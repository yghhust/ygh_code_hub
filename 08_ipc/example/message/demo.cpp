#include "demo.h"
#include <stdexcept>

void to_json(nlohmann::json& j, const AddRequest& obj) {
    j = nlohmann::json{
        {"a", obj.a},
        {"b", obj.b}
    };
}

void from_json(const nlohmann::json& j, AddRequest& obj) {
    j.at("a").get_to(obj.a);
    j.at("b").get_to(obj.b);
}

void to_json(nlohmann::json& j, const SubRequest& obj) {
    j = nlohmann::json{
        {"a", obj.a},
        {"b", obj.b}
    };
}

void from_json(const nlohmann::json& j, SubRequest& obj) {
    j.at("a").get_to(obj.a);
    j.at("b").get_to(obj.b);
}

void to_json(nlohmann::json& j, const AddResponse& obj) {
    j = nlohmann::json{
        {"result", obj.result}
    };
}

void from_json(const nlohmann::json& j, AddResponse& obj) {
    j.at("result").get_to(obj.result);
}

void to_json(nlohmann::json& j, const SubResponse& obj) {
    j = nlohmann::json{
        {"result", obj.result}
    };
}

void from_json(const nlohmann::json& j, SubResponse& obj) {
    j.at("result").get_to(obj.result);
}

void to_json(nlohmann::json& j, const calc_message& cmd) {
    std::visit([&j](const auto& obj) {
        nlohmann::json inner = obj;
        std::string cmd_name;
        if constexpr (std::is_same_v<std::decay_t<decltype(obj)>, AddRequest>) {
            cmd_name = "add_request";
        } else
        if constexpr (std::is_same_v<std::decay_t<decltype(obj)>, AddResponse>) {
            cmd_name = "add_response";
        } else
        if constexpr (std::is_same_v<std::decay_t<decltype(obj)>, SubRequest>) {
            cmd_name = "sub_request";
        } else
        if constexpr (std::is_same_v<std::decay_t<decltype(obj)>, SubResponse>) {
            cmd_name = "sub_response";
        } else
        {
            cmd_name = "unknown";
        }
        j = nlohmann::json{ {cmd_name, inner} };
    }, cmd);
}

void from_json(const nlohmann::json& j, calc_message& cmd) {
    if (j.contains("add_request")) {
        cmd = j.at("add_request").get<AddRequest>();
    } else if (j.contains("add_response")) {
        cmd = j.at("add_response").get<AddResponse>();
    } else if (j.contains("sub_request")) {
        cmd = j.at("sub_request").get<SubRequest>();
    } else if (j.contains("sub_response")) {
        cmd = j.at("sub_response").get<SubResponse>();
    } else {
        throw std::invalid_argument("JSON does not contain any known command key");
    }
}
