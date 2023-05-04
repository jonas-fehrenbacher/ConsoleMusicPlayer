#pragma once

#include "core/StateMachine.hpp"
#include "core/ScrollableList.hpp"
#include "core/MessageBus.hpp"
#include "core/Playlist.hpp"
#include <filesystem>
#include <vector>
#include <array>

class App;

/* Playlist all is a virtual playlist which plays all music there is. */
class MenuState : public core::State
{
public:
    struct Style
    {
        core::Color arrow;
        core::Color item;
        core::Color selected;
        core::Color hover;
        core::Color statusOn;
        core::Color statusOff;

        core::Color durationProgressBar;
        core::Color durationProgressBarText;
        core::Color durationText;
    };

    MenuState(App* app);

    void init() override;
    void terminate() override;
    void update() override;
    void handleEvent() override;
    void draw() override;
private:
    enum Option
    {
        AllMusic = 0,
        Playlists = 1,
        Directories = 2,

        None = 3, // If 'hover' is equal 'None', then we are currently in the music list, playlist list or directory list.
        First = 0,
        Last = 2
    };

    enum class Replay
    {
        None  = 0, // Loop deactivated
        One   = 1, // Loop currently playing music
        All   = 2, // Loop playlist - all music
        Count = 3
    };

    App*                       app;
    std::array<std::string, 3> options;
    Option                     selected; // this item is selected
    Option                     hover; // over this item do we hover
    core::ScrollableList       playlistList;
    core::ScrollableList       musicList;
    core::ScrollableList       directoryList;
    bool                       drawKeyInfo;
    Style                      style;
    bool                       firstInit;

    core::Playlist playlist;
    Replay         replay;
    core::Text     skipReport;
    core::Timer    cooldownSkipReport;
    core::Text     volumeReport;
    core::Timer    cooldownVolumeReport;

    void initMusicList();
    void initDirectories();
    void initPlaylists();
    void onMessage(core::Message message);
    bool isInsideNavBar();
    /** return nullptr if no list is active. */
    core::ScrollableList* getActiveList();
    void handlePlaylistEvents();
};