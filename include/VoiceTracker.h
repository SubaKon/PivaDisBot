#pragma once
#include <dpp/dpp.h>
#include "jsonSave.h"

std::string get_global_name(const dpp::voice_state_update_t& event);

void userStalker(JsonSave& userCheck,const  dpp::slashcommand_t& event);
class VoiceTracker {
public:
    void logOnVoiceStateUpdate(dpp::cluster& bot, const dpp::voice_state_update_t& event, const dpp::snowflake& logChanel,JsonSave& usersCheck);
private:
    std::map<dpp::snowflake, dpp::snowflake> previous_channel;  // user_id → channel_id
    void checkUser(dpp::cluster& bot, JsonSave& usersCheck, dpp::snowflake& user, std::string username);

};