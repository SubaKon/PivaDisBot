#include "LogInChanel.h"
using namespace std;

bool save_snowflake(const std::string& filename, dpp::snowflake id)
{
    std::ofstream file(filename);
    if (!file.is_open()) return false;

    file << static_cast<uint64_t>(id);   // просто число
    return true;
}

// Прочитать один snowflake из файла
std::optional<dpp::snowflake> load_snowflake(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open()) return std::nullopt;

    uint64_t value;
    if (file >> value) {
        return dpp::snowflake(value);
    }
    return std::nullopt;
}

dpp::snowflake loadChanelFromFile() {
    auto maybeBaby = load_snowflake("logChanel.dat");
    if (maybeBaby) {                                    // если что-то загрузилось
        dpp::snowflake logChannel = *maybeBaby;         // или maybe_id.value()
        cout << "Файл logChanel.dat прочитан\n";
        return logChannel;
    }
    cout << "Файл logChanel.dat не найден или пуст — канал не установлен\n";
    return dpp::snowflake(0);
}

void logChanelCreate(const dpp::slashcommand_t& event, dpp::snowflake& logChanel) {
    logChanel = get<dpp::snowflake>(event.get_parameter("chanel"));
    if (save_snowflake("logChanel.dat",logChanel)) {
        event.reply(dpp::message("Канал сохранен!").set_flags(dpp::m_ephemeral));
        return;
    }
    event.reply(dpp::message("Ошибка сохранения!").set_flags(dpp::m_ephemeral));
}

void logOnGuildMemberUpdate(dpp::cluster& bot, const dpp::guild_member_update_t& event, const dpp::snowflake& logChanel) {

};

void logOnGuildMemberAdd(dpp::cluster& bot, const dpp::guild_member_add_t& event, const dpp::snowflake& logChanel) {
    string message = "Подключился новый участник: " + event.added.get_nickname()+"\n";
    message+=event.added.get_user()->get_url();
    bot.message_create(dpp::message(logChanel,message));
}

void logOnGuildMemberRemove(dpp::cluster& bot, const dpp::guild_member_remove_t& event, const dpp::snowflake& logChanel) {
    string message = "Участник: " + event.removed.format_username()+" покинул сервер\n";
    message+=event.removed.get_url();
    bot.message_create(dpp::message(logChanel,message));
}

/*void logOnMessageDelete(dpp::cluster& bot, const dpp::message_delete_t& event, const dpp::snowflake& logChanel) {
    string message = " удалил сообщение из канала: "+ dpp::get_channel_cache()->find(event.channel_id)->name + "\n";
    message +=
    bot.message_create(dpp::message(logChanel,message));
}*/

