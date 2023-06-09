#pragma once
#include "core/DrawableList.hpp"
#include "core/StateMachine.hpp"
#include "core/MessageBus.hpp"

class App;

class DirectoryState : public core::State
{
public:
    DirectoryState(App* app);

    void init() override;
    void terminate() override;
    void update() override;
    void handleEvent() override;
    void draw() override;
private:
    App*                 app;
    core::DrawableList   list;
    long long            messageReceiverID;

    void onMessage(core::Message message);
};