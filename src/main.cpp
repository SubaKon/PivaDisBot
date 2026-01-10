#include <templatebot/templatebot.h>

//#include <fstream>
//#include <string>
//#include <windows.h>
//#include <thread>
//#include <atomic>
//#include <iostream>

//#include <algorithm> // ��� find
//#include <utility>  // ��� std::pair

#include "jsonSave.h"
#include "RoleRules.h"
#include "LogInChanel.h"
#include "VoiceTracker.h"
#include "MusicPlayer.h"

using namespace std;

int main()
{
	std::cout << "=== БОТ СТАРТУЕТ ===\n";
	dpp::json configdocument;
	std::ifstream configfile("../config.json");
	configfile >> configdocument;
	const std::string BOT_TOKEN = configdocument["token"];

	/* Create bot cluster */
	dpp::cluster bot(BOT_TOKEN, dpp::i_default_intents | dpp::i_guild_members | dpp::i_message_content | dpp::i_guild_voice_states);

	JsonSave db;           // ← теперь локальный объект!
	cout << "Объект db создан\n";
	db.load();
	cout << "db.load() прошёл успешно!\n";
	dpp::snowflake logChanel=loadChanelFromFile();

	JsonSave usersCheck;
	usersCheck.filename="UsersCheck.json";
	usersCheck.load();

	VoiceTracker voice_tracker;

	MusicPlayer player(bot);

	/* Output simple log messages to stdout */
	bot.on_log(dpp::utility::cout_logger());

	/* Handle slash command with the most recent addition to D++ features, coroutines! */
	bot.on_slashcommand([&db,&logChanel,&usersCheck,&player](const dpp::slashcommand_t& event) {
		if (event.command.get_command_name() == "ping") {
			uint64_t ms = (event.command.id >> 22)+1420070400000ULL;
			auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock::now().time_since_epoch()
				).count();
			event.reply("Pong! "+to_string(now-ms)+" ms");
		}

		if (event.command.get_command_name() == "rolerulsadd") {
			roleRulsAdd(db,event);
		}

		if (event.command.get_command_name() == "rolerulscheck") {
			roleRulsCheck(db,event);
		}

		if (event.command.get_command_name() == "rolerulsdel") {
			roleRulsDel(db,event);
		}
		if (event.command.get_command_name() == "logchanelcreate") {
			logChanelCreate(event,logChanel);
		}
		if (event.command.get_command_name() == "userstalker") {
			userStalker(usersCheck,event);
		}
		if (event.command.get_command_name() == "musicplayer") {
			player.createMusicPlayer(event);
		}
		if (event.command.get_command_name() == "join") {
			player.join(event);
		}

		return;
		});

	bot.on_button_click([&player](const dpp::button_click_t& event) {
	  /* Нажатия кнопок — это всё ещё взаимодействия, и на них нужно отвечать в какой-либо форме
       * предотвратить сообщение «это взаимодействие не удалось» от Discord пользователю.
	   */
	  	bool flag = false;
	  	if (event.custom_id =="beer") {
	  		player.play(event);
	  	}
		if (event.custom_id =="pause") {
			  player.pausse();
			  flag = true;
		}
		if (event.custom_id =="next") {
			  player.skip();
			  flag = true;
		}
		if (event.custom_id =="previous") {
			  //player.previous();
		}
		if (event.custom_id =="stop") {
			  player.stop();
			  flag = true;
		}
		if (flag) {
			event.reply(dpp::message("Успешно "+event.custom_id).set_flags(dpp::m_ephemeral));
		}
		else {
			//event.reply(dpp::message("Не, в другой раз...").set_flags(dpp::m_ephemeral));
		}

    });

	bot.on_guild_member_update([&bot, &db, &logChanel](const dpp::guild_member_update_t& event) {
		onGuildMemberUpdate(bot, db, event);
		logOnGuildMemberUpdate(bot, event, logChanel);
	});

	bot.on_guild_member_add([&bot,logChanel](const dpp::guild_member_add_t& event) {
		logOnGuildMemberAdd(bot,event,logChanel);
	});

	bot.on_guild_member_remove([&bot,logChanel](const dpp::guild_member_remove_t& event) {
		logOnGuildMemberRemove(bot,event,logChanel);
	});

	bot.on_voice_state_update([&bot,logChanel,&voice_tracker,&usersCheck](const dpp::voice_state_update_t& event) {
		voice_tracker.logOnVoiceStateUpdate(bot, event, logChanel,usersCheck);
	});

	//bot.on_message_delete([&bot,logChanel](const dpp::message_delete_t& event) {
		//logOnMessageDelete(bot,event,logChanel);
	//});
		/* Register slash command here in on_ready */
	bot.on_ready([&bot](const dpp::ready_t& event) {
		/* Wrap command registration in run_once to make sure it doesnt run on every full reconnection */
		if (dpp::run_once<struct register_bot_commands>()) {


			dpp::slashcommand pingPong("ping", "Ping pong!", bot.me.id);
			bot.global_command_create(pingPong);

			dpp::slashcommand roleRulsAdd("rolerulsadd", "Добавить исключающие роли", bot.me.id);
			roleRulsAdd.add_option(dpp::command_option(dpp::co_role, "master", "Роль исключитель", true));
			roleRulsAdd.add_option(dpp::command_option(dpp::co_role, "slave", "Роль что удаляется", true));
			roleRulsAdd.set_default_permissions(8);
			bot.global_command_create(roleRulsAdd);

			dpp::slashcommand roleRulsCheck("rolerulscheck", "Проверить исключающие роли", bot.me.id);
			roleRulsCheck.set_default_permissions(8);
			bot.global_command_create(roleRulsCheck);

			dpp::slashcommand roleRulsDel("rolerulsdel", "Удалить правило", bot.me.id);
			roleRulsDel.add_option(dpp::command_option(dpp::co_integer, "number", "Номер роли (/rolerulscheck)", true));
			roleRulsDel.set_default_permissions(8);
			bot.global_command_create(roleRulsDel);

			dpp::slashcommand logChanelCreate("logchanelcreate", "Установить канал для ведения лога", bot.me.id);
			logChanelCreate.add_option(dpp::command_option(dpp::co_channel, "chanel", "Канал для лога", true));
			logChanelCreate.set_default_permissions(8);
			bot.global_command_create(logChanelCreate);

			dpp::slashcommand userCheck("userStalker", "Сталкерить пявление пользователя в гс", bot.me.id);
			userCheck.add_option(dpp::command_option(dpp::co_user, "user", "Кого сталкерим", true));
			bot.global_command_create(userCheck);

			dpp::slashcommand musicPlayer("musicplayer","Создать музыкальный плеер", bot.me.id);
			musicPlayer.add_option(dpp::command_option(dpp::co_channel, "channel", "Где создать меню", true));
			bot.global_command_create(musicPlayer);

			dpp::slashcommand joincommand("join", "Подключиться к твоему каналу", bot.me.id);
			bot.global_command_create(joincommand);
		}
		});
	
	/* Start the bot */
	bot.start(dpp::st_wait);

	return 0;
}
