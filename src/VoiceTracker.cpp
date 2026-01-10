#include "VoiceTracker.h"
using namespace std;

std::string get_global_name(const dpp::voice_state_update_t& event){
    try {
        dpp::json json = dpp::json::parse(event.raw_event);  // парсим строку в JSON

        // Путь: d.member.user.global_name
        if (json.contains("d") && json["d"].contains("member") &&
            json["d"]["member"].contains("user") &&
            json["d"]["member"]["user"].contains("global_name") &&
            !json["d"]["member"]["user"]["global_name"].is_null()) {

            return json["d"]["member"]["user"]["global_name"].get<std::string>();
            }
    }
    catch (const std::exception& e) {
        // Если парсинг упал — просто молчим
        std::cout << "Ошибка парсинга raw_event: " << e.what() << "\n";
    }

    return "Неизвестно";  // fallback
}


void VoiceTracker::checkUser(dpp::cluster& bot, JsonSave& usersCheck, dpp::snowflake& user, std::string username) {
    int index = 0;
    for (const auto& [master, slave] : usersCheck.roleRules) {
        if (slave==user) {
            bot.direct_message_create(master,dpp::message(username + " в сети"));
            usersCheck.roleRules.erase(usersCheck.roleRules.begin() + index);
            usersCheck.save();  // сразу сохраняем в файл
        }
        index++;
    }
}

void VoiceTracker::logOnVoiceStateUpdate(
    dpp::cluster& bot, const dpp::voice_state_update_t& event, const dpp::snowflake& logChanel, JsonSave& usersCheck
    ) {
    dpp::snowflake user_id = event.state.user_id;
    dpp::snowflake newChannel_id = event.state.channel_id;
    dpp::snowflake old_channel = 0;
    string message;
    string userName = get_global_name(event);


    auto it = previous_channel.find(user_id);
    if (it != previous_channel.end()) {
        old_channel = it->second;
    }

    if (old_channel == 0 && newChannel_id != 0) {
        message =userName+" подключился к  "+dpp::find_channel(newChannel_id)->name;
        checkUser(bot,usersCheck,user_id,userName);

    }
    else if (old_channel != 0 && newChannel_id == 0) {
        message =userName+ " отключился из " + dpp::find_channel(old_channel)->name;
    }
    else if (old_channel != 0 && newChannel_id != 0 && old_channel != newChannel_id) {
        message = userName + " перешел из " + dpp::find_channel(old_channel)->name + " в " + dpp::find_channel(newChannel_id)->name;
    }

    bot.message_create(dpp::message(logChanel,message));
    // Обновляем мапу
    if (newChannel_id != 0) {
        previous_channel[user_id] = newChannel_id;
    } else {
        previous_channel.erase(user_id);
    }
}

void userStalker(JsonSave& userCheck,const  dpp::slashcommand_t& event) {
    userCheck.roleRules.emplace_back(
        event.command.get_issuing_user().id,
        std::get<dpp::snowflake>(event.get_parameter("user"))
    );
    userCheck.save();
    event.reply(dpp::message("Запомнил слежку").set_flags(dpp::m_ephemeral));
}