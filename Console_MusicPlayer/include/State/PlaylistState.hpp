#pragma once

#include "core/ScrollableList.hpp"

class App;
namespace core {
    class MusicPlayer;
}

class PlaylistState
{
public:
    enum class State
    {
        Playlist,
        PlaylistList
        // TODO Both
    };

    void init(App* app, core::MusicPlayer* musicPlayer);
    void terminate();
    void update();
    void handleEvent();
    void draw();
    void start();
    void loseFocus();
    void gainFocus();
    bool isTrappedOnTop();
    void scrollToTop();
    void onConsoleResize();
    void setDrawKeyInfo(bool drawKeyInfo);
private:
    App*                 app;
    //core::Playlist       playlist; // currently playing / active
    //core::Playlist       playlistView; // currently visible on the screen, may not play music - maybe use core::ScrollableList
    core::MusicPlayer*   musicPlayer;
    core::ScrollableList playlistList;
    State                state;
    bool                 drawKeyInfo;
};