#pragma once
#include <dpp/dpp.h>
#include <queue>
#include <string>

struct Track {
    std::string filename;  // путь к MP3
    std::string title;     // название
};

class MusicPlayer {
public:
    explicit MusicPlayer(dpp::cluster& b);

    void createMusicPlayer(const  dpp::slashcommand_t& event);
    void join(const dpp::button_click_t& event);
    void join(const dpp::slashcommand_t& event);

    void add_track(const std::string& filename, const std::string& title = "");
    void play(const dpp::button_click_t& event);  // без guild_id
    void skip();
    void stop();
    void pausse();
    void resume();

    dpp::message editMusicPlayer(const std::string& status = "Очередь пуста");

private:
    dpp::cluster& bot;
    dpp::slashcommand_t Event;

    std::queue<Track> queue;
    bool is_paused = false;
    bool skip_requested = false;

    dpp::snowflake control_message_id = 0;
    dpp::snowflake control_channel_id = 0;

    dpp::voiceconn* voice_connection = nullptr;

    void update_control_panel(const std::string& status);
    void stream_next_internal();
    void start_playback();

    std::thread playback_thread;
    std::atomic<bool> stop_playback{false};     // атомарный флаг для безопасного прерывания
    bool is_playing = false;

};