#include "MusicPlayer.h"
#include <mpg123.h>
#include <dpp/unicode_emoji.h>
#include <thread>
#include <chrono>
#include <filesystem>
#include <dpp/voicestate.h>

MusicPlayer::MusicPlayer(dpp::cluster& b) : bot(b) {
    mpg123_init();  // инициализируем mpg123 один раз

    const std::string music_folder = "/home/subakon/Музыка/2.klas";  // ← замени на свой путь, если нужно

    if (!std::filesystem::exists(music_folder)) {
        std::cout << "[MusicPlayer] Папка " << music_folder << " не найдена!\n";
        return;
    }

    std::cout << "[MusicPlayer] Сканирую папку " << music_folder << "...\n";

    for (const auto& entry : std::filesystem::directory_iterator(music_folder)) {
        if (entry.is_regular_file()) {
            std::string filename = entry.path().string();
            std::string extension = entry.path().extension().string();

            // Поддерживаем .mp3 (можно добавить .ogg, .wav и т.д.)
            if (extension == ".mp3" || extension == ".MP3") {
                add_track(filename);  // используем твою функцию
                std::cout << "[MusicPlayer] Добавлен трек: " << entry.path().filename().string() << "\n";
            }
        }
    }

    std::cout << "[MusicPlayer] Загружено " << queue.size() << " треков.\n";
}

void MusicPlayer::createMusicPlayer(const  dpp::slashcommand_t& event) {
    dpp::message msg = editMusicPlayer("Плеер только что создан");
    bot.message_create(msg.set_channel_id(event.command.channel_id), [this](const dpp::confirmation_callback_t& cb) {
            if (!cb.is_error()) {
                control_message_id = cb.get<dpp::message>().id;  // ← СОХРАНЯЕМ ID!
                control_channel_id = cb.get<dpp::message>().channel_id;
                std::cout << "Панель создана, ID: " << control_message_id << "\n";
            }
        });
    event.reply("Панель управления музыкой создана!");
}

dpp::message MusicPlayer::editMusicPlayer(const std::string& status) {
    /* Create a message */
    //dpp::snowflake channel =std::get<dpp::snowflake>(event.get_parameter("channel"));
    dpp::message msg(status);

    /* Add an action row, and then a button within the action row. */
    msg.add_component(
        dpp::component().add_component(
            dpp::component()
                //.set_label("Click me!")
                .set_type(dpp::cot_button)
                .set_emoji(dpp::unicode_emoji::previous_track)
                //.set_style(dpp::cos_secondary)
                .set_id("previous")
        )

        .add_component(
            dpp::component()
                //.set_label("Click me!")
                .set_type(dpp::cot_button)
                .set_emoji(dpp::unicode_emoji::beer)
                .set_style(dpp::cos_secondary)
                .set_id("beer")
        )

        .add_component(
            dpp::component()
                //.set_label("Click me!")
                .set_type(dpp::cot_button)
                .set_emoji(dpp::unicode_emoji::pause_button)
                .set_style(dpp::cos_secondary)
                .set_id("pause")
        )

        .add_component(
            dpp::component()
                //.set_label("Click me!")
                .set_type(dpp::cot_button)
                .set_emoji(dpp::unicode_emoji::next_track)
                //.set_style(dpp::cos_danger)
                .set_id("next")
        )
        .add_component(
            dpp::component()
                //.set_label("Click me!")
                .set_type(dpp::cot_button)
                .set_emoji(dpp::unicode_emoji::stop_button)
                .set_style(dpp::cos_danger)
                .set_id("stop")
        )
    );

    /* Reply to the user with our message. */
    return msg;
}
void MusicPlayer::join(const dpp::button_click_t& event) {
    /* Get the guild */
    dpp::guild* g = dpp::find_guild(event.command.guild_id);

    /* Attempt to connect to a voice channel, returns false if we fail to connect. */
    if (!g->connect_member_voice(*event.owner, event.command.get_issuing_user().id)) {
        event.reply("Пива набухалась сама, и не придёт");
        return;
    }
    /* Tell the user we joined their channel. */
    event.reply("Пива подключилась к тебе!");
}

void MusicPlayer::join(const dpp::slashcommand_t& event) {
    /* Get the guild */
    dpp::guild* g = dpp::find_guild(event.command.guild_id);

    /* Attempt to connect to a voice channel, returns false if we fail to connect. */
    if (!g->connect_member_voice(*event.owner, event.command.get_issuing_user().id)) {
        event.reply("Пива набухалась сама, и не придёт");
        return;
    }
    /* Tell the user we joined their channel. */
    Event=event;
    event.reply("Пива подключилась к тебе!");
}



void MusicPlayer::stream_next() {
    // 1. Проверяем, есть ли что играть
    if (queue.empty()) {
        update_control_panel("Очередь пуста");
        is_paused = false;  // музыка закончилась
        std::cout << "[STREAM] Очередь пуста — воспроизведение остановлено\n";
        return;
    }

    // 2. Берём следующий трек из очереди
    Track current = queue.front();
    queue.pop();  // удаляем его из очереди

    // 3. Обновляем статус (кнопки, текст)
    std::cout << "[STREAM] Начинаю играть: " << current.title << " (" << current.filename << ")\n";
    update_control_panel("Играет: " + current.title);

    // 4. Получаем голосовое соединение бота
    std::cout << "[VOICE] Пытаюсь получить voiceconn...\n";
    dpp::voiceconn* vc = voice_connection;
    std::cout << "[VOICE] vc указатель: " << vc << "\n";

    if (!vc) {
        std::cout << "[VOICE] Ошибка: vc == nullptr — бот не в голосовом!\n";
        return;
    }

    std::cout << "[VOICE] voiceclient указатель: " << vc->voiceclient << "\n";
    if (!vc->voiceclient) {
        std::cout << "[VOICE] Ошибка: voiceclient == nullptr\n";
        return;
    }

    bool ready = vc->voiceclient->is_ready();
    std::cout << "[VOICE] is_ready(): " << (ready ? "true" : "false") << "\n";

    if (!ready) {
        std::cout << "[VOICE] Соединение НЕ готово к отправке аудио — выходим\n";
        return;
    }

    std::cout << "[VOICE] Голосовое соединение готово — начинаю декодирование MP3\n";

    // 5. Открываем MP3
    mpg123_handle* mh = mpg123_new(nullptr, nullptr);
    std::cout << "[MP3] Открываю файл: " << current.filename << "\n";
    std::cout << "[MP3] Файл существует: " << std::filesystem::exists(current.filename) << "\n";

    int open_result = mpg123_open(mh, current.filename.c_str());
    if (open_result != MPG123_OK) {
        std::cout << "[MP3] Ошибка открытия файла: " << mpg123_strerror(mh) << " (код: " << open_result << ")\n";
        mpg123_delete(mh);
        stream_next();  // следующий трек
        return;
    }

    // 6. Принудительно устанавливаем нужный формат для Discord
    mpg123_param(mh, MPG123_FORCE_RATE, 48000, 48000.0);           // 48 кГц
    mpg123_format(mh, 48000, MPG123_STEREO, MPG123_ENC_16);  // стерео, 16-бит

    // 7. Буфер для чтения аудио
    unsigned char buffer[12000];  // ~20–40 мс аудио за раз

    int read_result;
    size_t done;
    int packets_sent = 0;

    std::cout << "[MP3] Начинаю цикл чтения...\n";

    while ((read_result = mpg123_read(mh, buffer, sizeof(buffer), &done)) == MPG123_OK || read_result == MPG123_NEW_FORMAT) {
        if (read_result == MPG123_NEW_FORMAT) {
            std::cout << "[MP3] Новый формат обнаружен — продолжаю...\n";
            continue;  // просто ждём следующий вызов
        }

        if (is_paused) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        if (skip_requested) {
            skip_requested = false;
            std::cout << "[CONTROL] Скип — прерываю трек\n";
            break;
        }

        packets_sent++;
        //std::cout << "[AUDIO] Отправляю пакет #" << packets_sent << " — " << done << " байт\n";

        vc->voiceclient->send_audio_raw((uint16_t*)buffer, done);
        //std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    std::cout << "[MP3] Трек закончился. Отправлено пакетов: " << packets_sent << "\n";

    // После цикла — проверяем, почему вышли
    if (read_result != MPG123_DONE && read_result != MPG123_OK) {
        std::cout << "[MP3] Ошибка чтения: " << mpg123_strerror(mh)
                  << " (код: " << read_result << ")\n";
    } else if (read_result == MPG123_DONE) {
        std::cout << "[MP3] Трек закончился нормально\n";
    }

    // 9. Закрываем файл
    mpg123_close(mh);
    mpg123_delete(mh);

    // 10. Автоматически запускаем следующий трек
    std::cout << "[STREAM] Запускаю следующий трек...\n";
    stream_next();  // ← рекурсия! После конца текущего — сразу следующий
}

void MusicPlayer::add_track(const std::string& filename, const std::string& title) {
    std::string display_title = title;
    if (display_title.empty()) {
        display_title = std::filesystem::path(filename).filename().stem().string();
    }
    queue.push({filename, display_title});

    // Если ничего не играет — начинаем сразу
    if (queue.size() == 1 && is_paused) {
        stream_next();
    }
}

void MusicPlayer::play(const dpp::button_click_t& event) {
    join(event);
    voice_connection=event.from()->get_voice(event.command.guild_id);
    std::cout << "voice_connection "<< voice_connection << "\n";
    is_paused = false;
    if (!queue.empty()) {
        stream_next();
    }
}  // без guild_id

void MusicPlayer::skip() {
    if (queue.empty() && is_paused) {
        return;  // ничего не играет — нечего скипать
    }
    skip_requested = true;  // ставим флаг
    update_control_panel("Пропуск трека...");
}
void MusicPlayer::stop() {
    queue = std::queue<Track>{};  // очищаем очередь
    is_paused = true;
    update_control_panel("Остановлено");
}
void MusicPlayer::pausse() {
    is_paused = true;
    update_control_panel("Пивная пауза");
}
void MusicPlayer::resume() {
    is_paused = false;
    update_control_panel("Играет");
}
void MusicPlayer::update_control_panel(const std::string& status) {
    if (control_message_id == 0) return;
    dpp::message msg =editMusicPlayer(status);
    msg.id=control_message_id;
    msg.channel_id=control_channel_id;
    bot.message_edit(msg);
}