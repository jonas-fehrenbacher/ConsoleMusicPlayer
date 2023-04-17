#pragma once

#include "State/StateMachine.hpp"
#include "Timer/Timer.hpp"
#include "Music/Playlist.hpp"
#include "Tools/ColoredStr.hpp"

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
private:
    App*             app;
    core::ColoredStr volumeReport;
    core::Timer      cooldownVolumeReport;
    core::ColoredStr skipReport;
    core::Timer      cooldownSkipReport;
    core::Time       totalRunTime;
    core::Timer      totalRunTimeClock;
    core::Playlist   playlist; 
    bool             isPrintOrder; 
    bool             isPrintKeyInfo;
};