#pragma once
#include <string>
#include <vector>
#include <dpp/dpp.h>
//#include <nlohmann/json.hpp>

using json = nlohmann::ordered_json;

namespace ns {
    struct Role {
        dpp::snowflake master;
        dpp::snowflake slave;

        // ← ЭТОТ КОНСТРУКТОР РЕШАЕТ ВСЁ!
        Role(dpp::snowflake m, dpp::snowflake s) : master(m), slave(s) { }

        // Можно ещё добавить конструктор по умолчанию (на всякий случай)
        Role() = default;
    };

    void to_json(json& j, const Role& val);
    void from_json(const json& j, Role& val);
}

class JsonSave{
public:
    std::string filename = "role_rules.json";

    // Вот оно — теперь это ЧЛЕН класса!
    std::vector<ns::Role> roleRules;

    void load();
    void save();
};