#pragma once

#include "State/StateMachine.hpp"
#include <filesystem>
#include <vector>
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
    App*                     app;
    std::vector<std::string> options;
    int                      selected;
    core::ScrollableList     musicList;
};