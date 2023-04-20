#pragma once

#include "State/StateMachine.hpp"
#include <filesystem>
class App;
namespace fs = std::filesystem;

class PlaylistEditorState : public core::State
{
public:
    PlaylistEditorState(App* app);

    void init() override;
    void terminate() override;
    void update() override;
    void handleEvent() override;
    void draw() override;
private:
    App*                  app;
    std::vector<fs::path> musicDirs; // in this directories I search for music.
};