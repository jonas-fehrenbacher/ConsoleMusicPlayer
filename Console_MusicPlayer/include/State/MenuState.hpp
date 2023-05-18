#pragma once

#include "core/StateMachine.hpp"
#include "core/ScrollableList.hpp"
#include "core/MessageBus.hpp"
#include "core/MusicPlayer.hpp"
#include "State/PlaylistState.hpp"
#include "State/DirectoryState.hpp"
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

    App*                       app;
    std::array<std::string, 3> options;
    Option                     selected; // this item is selected
    Option                     hover; // over this item do we hover
    bool                       drawKeyInfo;
    Style                      style;
    bool                       firstInit;
    PlaylistState              playlistState;
    DirectoryState             directoryState;
    core::MusicPlayer          musicPlayer;
    
    core::Time     playbackDuration;

    void onMessage(core::Message message);
    bool isInsideNavBar();
};