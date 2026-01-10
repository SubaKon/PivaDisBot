// JsonSave.cpp
#include "jsonSave.h"
/*#include <iostream>
#include <fstream>
#include <iomanip>*/

using json = nlohmann::ordered_json;


namespace ns {

    /*
    struct Role
    {
        dpp::snowflake master;
        dpp::snowflake slave;
    };*/

    void from_json(const json& j, ns::Role& val)
    {
        j.at("Master").get_to(val.master);
        j.at("Slave").get_to(val.slave);
    }

    void to_json(json& j, const ns::Role& val)
    {
        j["Master"] = val.master.operator uint64_t();
        j["Slave"] = val.slave.operator uint64_t();;
    }
} // namespace ns

//std::vector<ns::Role> roleRules;

void JsonSave::load()
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "Файл не найден — будет создан новый при первом сохранении.\n";
        return;   // roleRules остаётся пустым
    }

    json j;
    try {
        file >> j;   // читаем весь файл в JSON-объект
    }
    catch (...) {
        std::cout << "Файл повреждён — начинаем с чистого листа.\n";
        return;
    }

    // Магия: благодаря from_json() превращаем JSON обратно в вектор!
    roleRules = j.get<std::vector<ns::Role>>();

    std::cout << "Загружено " << roleRules.size() << " правил из " << filename << '\n';
}

void JsonSave::save() {
    // 1. Берём весь вектор roleRules и превращаем его в JSON
    //    Благодаря функции to_json() в namespace ns — это работает автоматически!
    json j = roleRules;
    //    После этой строки j содержит:
    //    [
    //        { "Master": 123456789, "Slave": 987654321 },
    //        { "Master": 111111111, "Slave": 222222222 }
    //    ]

    // 2. Открываем файл на запись
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cout << "Не удалось открыть файл!\n";
        return;
    }

    // 3. Пишем красиво отформатированный JSON в файл
    file << std::setw(4) << j << std::endl;
    //    std::setw(4) — делает отступы в 4 пробела (читаемо)

    std::cout << "Сохранено " << roleRules.size() << " правил в " << filename << '\n';
}
