//
// Created by subakon on 07.12.2025.
//

#pragma once
#include <dpp/dpp.h>


// Записать один snowflake в файл
bool save_snowflake(const std::string& filename, dpp::snowflake id);

// Прочитать один snowflake из файла
std::optional<dpp::snowflake> load_snowflake(const std::string& filename);

dpp::snowflake loadChanelFromFile();

void logChanelCreate(const dpp::slashcommand_t& event, dpp::snowflake& logChanel);

void logOnGuildMemberUpdate(dpp::cluster& bot, const dpp::guild_member_update_t& event, const dpp::snowflake& logChanel);
void logOnGuildMemberAdd(dpp::cluster& bot, const dpp::guild_member_add_t& event, const dpp::snowflake& logChanel);
void logOnGuildMemberRemove(dpp::cluster& bot, const dpp::guild_member_remove_t& event, const dpp::snowflake& logChanel);
void logOnMessageDelete(dpp::cluster& bot, const dpp::message_delete_t& event, const dpp::snowflake& logChanel);