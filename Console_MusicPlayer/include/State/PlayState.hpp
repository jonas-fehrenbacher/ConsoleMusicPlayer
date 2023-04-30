#pragma once

#include "core/StateMachine.hpp"
#include "core/Timer.hpp"
#include "core/Playlist.hpp"
#include "core/ColoredStr.hpp"
#include "core/ScrollableList.hpp"
#include "core/MessageBus.hpp"

class App;

class PlayState : public core::State
{
public:
    PlayState(App* app);

    void init() override;
    void terminate() override;
    void update() override;
    void handleEvent() override;
    void draw() override;

    void setPlaylist(fs::path playlistPath);
private:
    App*                 app;
    core::ColoredStr     volumeReport;
    core::Timer          cooldownVolumeReport;
    core::ColoredStr     skipReport;
    core::Timer          cooldownSkipReport;
    core::Time           totalRunTime;
    core::Timer          totalRunTimeClock;
    core::Playlist       playlist;
    core::ScrollableList musicList;
    bool                 drawKeyInfo;
    fs::path             playlistPath; // selected in menu

    void onMessage(core::Message message);
};