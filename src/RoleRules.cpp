#include "RoleRules.h"
using namespace std;

void roleRulsAdd( JsonSave& db,const  dpp::slashcommand_t& event) {
    db.roleRules.emplace_back(
        std::get<dpp::snowflake>(event.get_parameter("master")),
        std::get<dpp::snowflake>(event.get_parameter("slave"))
    );
    db.save();
    event.reply(dpp::message("Новое правило добавлено").set_flags(dpp::m_ephemeral));
};


void roleRulsDel(JsonSave& db, const dpp::slashcommand_t& event) {
    int index = get<int64_t>(event.get_parameter("number"))-1;
    if (index < 0) {
        event.reply(dpp::message("Номер должен быть больше 0").set_flags(dpp::m_ephemeral));
        return;
    }
    if (index >= db.roleRules.size()) {
        event.reply(dpp::message("Такого номера в списке нет").set_flags(dpp::m_ephemeral));
        return;  // индекс вне диапазона — ничего не делаем
    }
    db.roleRules.erase(db.roleRules.begin() + index);
    db.save();  // сразу сохраняем в файл
    event.reply(dpp::message("Правило удалено").set_flags(dpp::m_ephemeral));
    return;
};


void roleRulsCheck(JsonSave& db, const dpp::slashcommand_t& event) {
    string replymessage = "";
    int i =0;
    for (const auto& [master, slave] : db.roleRules) {
        i++;
        replymessage += "[" + to_string(i) + "] " + dpp::find_role(master)->name + " -> " + dpp::find_role(slave)->name + "\n";
    }
    if (replymessage=="") {
        event.reply(dpp::message("Правил нет").set_flags(dpp::m_ephemeral));
        return;
    }
    event.reply(dpp::message(replymessage).set_flags(dpp::m_ephemeral));
};

void onGuildMemberUpdate(dpp::cluster& bot,JsonSave& db, const dpp::guild_member_update_t& event) {
    bool has_role = false;
    for (const auto& [master, slave] : db.roleRules) {
        if (std::find(event.updated.get_roles().begin(), event.updated.get_roles().end(), master) != event.updated.get_roles().end()) {
            if (std::find(event.updated.get_roles().begin(), event.updated.get_roles().end(), slave) != event.updated.get_roles().end()) {
                bot.guild_member_remove_role(event.updating_guild.id, event.updated.user_id, slave,
                    [&](const dpp::confirmation_callback_t& confirmation) {
                        if (confirmation.is_error()) {
                            cout << "Error: " << confirmation.get_error().message << endl;
                        }
                        else {
                            cout << "Poshol nahui ueban!" << endl;
                        }
                    });
                }
            }
    }
};
