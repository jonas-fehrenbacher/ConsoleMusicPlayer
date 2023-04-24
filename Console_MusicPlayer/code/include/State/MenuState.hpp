#pragma once

#include "State/StateMachine.hpp"
#include <filesystem>
#include <vector>
#include <array>
#include "ScrollableList.hpp"

class App;

class MenuState : public core::State
{
public:
    MenuState(App* app);

    void init() override;
    void terminate() override;
    void update() override;
    void handleEvent() override;
    void draw() override;
    /* Playlist all is a virtual playlist which plays all music there is. */
    std::filesystem::path getPlaylistPath();
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

    void initMusicList();
    void initDirectories();
    bool isInsideOption();
    bool isInsideOption(Option option);
};