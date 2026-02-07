#pragma once
#include <dpp/dpp.h>
#include <queue>
#include <string>

#include <atomic>
#include <thread>

struct Track {
    std::string filename;  // путь к MP3
    std::string title;     // название
};

class MusicPlayer {
public:
    explicit MusicPlayer(dpp::cluster& b);

    // Подключаемся к голосовому каналу вызывающего (slash или button)
    void join(const dpp::slashcommand_t& event);

    void add_track(const std::string& filename, const std::string& title = "");

    void play();
    void stop();
    void pausa();

    bool is_playing() const { return playing.load(); }

    void createMusicPlayer(const  dpp::slashcommand_t& event);
    std::string scan();  // ← теперь принимает event, чтобы ответить
;
private:
    dpp::cluster& bot;

    dpp::voiceconn* voice_connection = nullptr;

    std::queue<Track> queue;
    std::atomic<bool> playing{false};
    std::atomic<bool> should_stop{false};

    std::thread playback_thread;

    //void playback_loop();
    void stream_next_internal();

    dpp::message editMusicPlayer(const std::string& status);
    void update_control_panel(const std::string& status);
    dpp::snowflake control_message_id;
    dpp::snowflake control_channel_id;
};