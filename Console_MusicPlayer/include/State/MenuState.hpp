#pragma once

#include "core/StateMachine.hpp"
#include "core/ScrollableList.hpp"
#include "core/SmallMusicPlayer.hpp"
#include "core/MessageBus.hpp"
#include <filesystem>
#include <vector>
#include <array>

class App;

/* Playlist all is a virtual playlist which plays all music there is. */
class MenuState : public core::State
{
public:
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

    App*                       app;
    std::array<std::string, 3> options;
    Option                     selected; // this item is selected
    Option                     hover; // over this item do we hover
    core::ScrollableList       playlistList;
    core::ScrollableList       musicList;
    core::ScrollableList       directoryList;
    core::SmallMusicPlayer     smallMusicPlayer;
    bool                       drawKeyInfo;

    void initMusicList();
    void initDirectories();
    void onMessage(core::Message message);
    bool isInsideOption();
    bool isInsideOption(Option option);
    int getMusicPlayerPosX();
};