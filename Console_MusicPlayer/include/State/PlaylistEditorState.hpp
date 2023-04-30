#pragma once

#include "core/StateMachine.hpp"
class App;

// This is still work in progress and currently stopped.

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
    App* app;
};