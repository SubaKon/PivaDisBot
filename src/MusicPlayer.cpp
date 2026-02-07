#include "MusicPlayer.h"
#include <mpg123.h>
#include <dpp/unicode_emoji.h>
#include <thread>
#include <chrono>
#include <filesystem>
//#include <dpp/voicestate.h>

MusicPlayer::MusicPlayer(dpp::cluster& b) : bot(b) {
    mpg123_init();  // инициализируем mpg123 один раз
    std::cout << scan() << std::endl;
}
std::string MusicPlayer::scan() {
    const std::string music_folder = "/home/subakon/Музыка";  // потом вынеси в конфиг!

    if (!std::filesystem::exists(music_folder) || !std::filesystem::is_directory(music_folder)) {
        return  "Папка с музыкой не найдена или это не папка: " + music_folder;
    }

    // Очищаем старую очередь перед новым сканом (самый простой способ)
    while (!queue.empty()) queue.pop();

    size_t added = 0;

    for (const auto& entry : std::filesystem::recursive_directory_iterator(music_folder)) {
        if (entry.is_regular_file()) {
            std::string path = entry.path().string();
            std::string ext = entry.path().extension().string();

            // Поддерживаем mp3 (можно добавить .ogg, .wav, .flac позже)
            if (ext == ".mp3" || ext == ".MP3") {
                add_track(path);
                added++;
            }
        }
    }

    std::string reply_text = "Сканирование завершено!\n";
    reply_text += "Найдено и добавлено треков: **" + std::to_string(added) + "**\n";
    reply_text += "Очередь очищена и заполнена заново.\n";
    if (added == 0) {
        reply_text += "⚠️ В папке нет .mp3 файлов (или доступ запрещён)";
    } else {
        reply_text += "Теперь можно жать Play или использовать кнопки!";
    }

    return reply_text;

    // Опционально: если ничего не играет — можно сразу стартануть
    // if (added > 0 && !playing) play();
}
void MusicPlayer::add_track(const std::string& filename, const std::string& title) {
    std::string t = title.empty() ? std::filesystem::path(filename).stem().string() : title;
    queue.push({filename, t});
    std::cout << "[Queue] + " << t << "\n";

    // Если ничего не играет — стартуем
    /*if (!playing && !queue.empty()) {
        play();
    }*/
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

void MusicPlayer::join(const dpp::slashcommand_t& event) {
    /* Get the guild */
    dpp::guild* g = dpp::find_guild(event.command.guild_id);

    /* Attempt to connect to a voice channel, returns false if we fail to connect. */
    if (!g->connect_member_voice(*event.owner, event.command.get_issuing_user().id)) {
        event.reply(dpp::message("Пива набухалась сама, и не придёт").set_flags(dpp::m_ephemeral));
        return;
    }
    /* Tell the user we joined their channel. */
    voice_connection=event.from()->get_voice(event.command.guild_id);
    event.reply(dpp::message("Пива подключилась к тебе!").set_flags(dpp::m_ephemeral));
}

/*void MusicPlayer::start_playback() {
    if (is_playing || queue.empty()) return;
    stop_playback = false;

    std::cout << "Пробуем стартануть поток\n";
    playback_thread = std::thread([this]() {
        while (!queue.empty() && !stop_playback && !is_playing) {
            is_playing = true;
            stream_next_internal();
        }
        is_playing = false;
        if (queue.empty()) {
            update_control_panel("Очередь пуста");
        }
    });
    playback_thread.detach();  // или join(), если хочешь ждать
}*/



void MusicPlayer::stop() {
    should_stop = true;
    playing = false;
}
void MusicPlayer::play() {
    if (playing || queue.empty()) return;

    auto vc = voice_connection;  // т.к. приватный сервер — берём любой, или сохрани snowflake в поле
    if (!vc || !vc->voiceclient || !vc->voiceclient->is_ready()) {
        std::cout << "[Play] Голос не готов или nullptr\n";
        return;
    }

    playing = true;
    should_stop = false;

    stream_next_internal();
    //playback_loop();
    //playback_thread = std::thread(&MusicPlayer::playback_loop, this);
    //playback_thread.detach();  // пока так, потом можно сделать joinable если нужно

    update_control_panel("Играет");
}
void MusicPlayer::pausa() {
    playing = false;
    update_control_panel("Пивная пауза");
}

/*void MusicPlayer::playback_loop() {
    constexpr long TARGET_RATE = 48000;
    //constexpr int TARGET_CHANNELS = MPG123_STEREO;
    //constexpr int TARGET_ENCODING = MPG123_ENC_SIGNED_16;
    //constexpr size_t SAMPLES_PER_PACKET = 960;             // 20 мс @ 48 кГц
    //constexpr size_t BYTES_PER_PACKET = SAMPLES_PER_PACKET * 4;  // 2 канала × 2 байта

    while (!queue.empty() && !should_stop) {
        Track track = queue.front();
        queue.pop();
        std::cout << "[Play] → " << track.title << "\n";

        int err = 0;
        mpg123_handle* mh = mpg123_new(NULL, &err);
        if (!mh) continue;

        // Жёстко заставляем нужный формат
        mpg123_param(mh, MPG123_FORCE_RATE, TARGET_RATE, TARGET_RATE);
        //mpg123_format_none(mh);
        //mpg123_format(mh, TARGET_RATE, TARGET_CHANNELS, TARGET_ENCODING);

        if (mpg123_open(mh, track.filename.c_str()) != MPG123_OK) {
            std::cout << "[MPG123] Не открылся: " << mpg123_strerror(mh) << "\n";
            mpg123_delete(mh);
            continue;
        }

        // Проверяем, что формат реально 48k/стерео/16bit
        long rate;
        int channels, encoding;
        mpg123_getformat(mh, &rate, &channels, &encoding);
        std::cout << "[Format] " << rate << " Hz, ch:" << channels << ", enc:" << encoding << "\n";

        size_t buffer_size = mpg123_outblock(mh);
        unsigned char* buffer = new unsigned char[buffer_size];
        size_t done = 0;
        auto vc = voice_connection;

        int read_result , packets_sent;
        while ((read_result = mpg123_read(mh, buffer, buffer_size, &done)) == MPG123_OK || read_result == MPG123_NEW_FORMAT) {
            if (read_result == MPG123_NEW_FORMAT) {
                std::cout << "[MP3] Новый формат обнаружен — продолжаю...\n";
                continue;  // просто ждём следующий вызов
            }

            packets_sent++;
            //std::cout << "[AUDIO] Отправляю пакет #" << packets_sent << " — " << done << " байт\n";

            vc->voiceclient->send_audio_raw((uint16_t*)buffer, done);
            std::this_thread::sleep_for(std::chrono::milliseconds(0));
        }



        mpg123_close(mh);
        mpg123_delete(mh);
        delete[] buffer;
        std::cout << "[End] " << track.title << "\n";
        std::cout << "[STREAM] Жду запуска следуюущего трека...\n";
    }

    playing = false;
    std::cout << "[Queue empty or stopped]\n";
}*/

void MusicPlayer::stream_next_internal() {
    Track current = queue.front();
    queue.pop();

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

    std::cout << "[VOICE] voiceclient указатель: " << vc->voiceclient.get() << "\n";
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
    int err = 0;
    mpg123_handle* mh = mpg123_new(NULL, &err);
    std::cout << "[MP3] Открываю файл: " << current.filename << "\n";
    std::cout << "[MP3] Файл существует: " << std::filesystem::exists(current.filename) << "\n";

    // 6. Принудительно устанавливаем нужный формат для Discord
    mpg123_param(mh, MPG123_FORCE_RATE, 48000, 48000);           // 48 кГц
    //mpg123_format(mh, 44000, MPG123_STEREO, MPG123_ENC_16);  // стерео, 16-бит
    mpg123_format_none(mh);
    mpg123_format(mh, 48000, MPG123_STEREO, MPG123_ENC_SIGNED_16);

    // 7. Буфер для чтения аудио
    size_t buffer_size = mpg123_outblock(mh);
    unsigned char* buffer = new unsigned char[buffer_size];;  // ~20–40 мс аудио за раз

    int channels, encoding;
    long rate;

    mpg123_open(mh, current.filename.c_str());
    mpg123_getformat(mh, &rate, &channels, &encoding);

    int read_result;
    size_t done =0;
    std::cout << "[MP3] Начинаю цикл чтения...\n";

    int ret;

    while ((ret = mpg123_read(mh, buffer, buffer_size, &done)) == MPG123_OK ||
            ret == MPG123_NEW_FORMAT) {
        if (ret == MPG123_NEW_FORMAT) continue;

        if (done == 0) break;

        // Добиваем тишиной, если конец файла
        if (done < buffer_size) {
            std::memset(buffer + done, 0, buffer_size - done);
            done = buffer_size;
        }

        vc->voiceclient->send_audio_raw(reinterpret_cast<uint16_t*>(buffer), done);

        // САМОЕ ВАЖНОЕ — тайминг!
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
            }

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
    delete[] buffer;

    // 10. Автоматически запускаем следующий трек
    std::cout << "[STREAM] Жду запуска следуюущего трека...\n";
}

void MusicPlayer::update_control_panel(const std::string& status) {
    if (control_message_id == 0) return;
    dpp::message msg =editMusicPlayer(status);
    msg.id=control_message_id;
    msg.channel_id=control_channel_id;
    bot.message_edit(msg);
}