#pragma once

#include "State/StateMachine.hpp"
class App;

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