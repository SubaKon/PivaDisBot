//
// Created by subakon on 07.12.2025.
//
#pragma once
#include <dpp/dpp.h>
#include  "jsonSave.h"

void roleRulsAdd(JsonSave& db, const dpp::slashcommand_t& event);
void roleRulsDel(JsonSave& db, const dpp::slashcommand_t& event);
void roleRulsCheck(JsonSave& db, const dpp::slashcommand_t& event);
void onGuildMemberUpdate(dpp::cluster& bot,JsonSave& db, const dpp::guild_member_update_t& event);