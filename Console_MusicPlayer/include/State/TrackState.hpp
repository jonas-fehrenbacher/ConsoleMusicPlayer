#pragma once

#include "core/StateMachine.hpp"
#include "core/MessageBus.hpp"
#include "core/Console.hpp"

class App;

class TrackState : public core::State
{
public:
    struct Style
    {

    };

    TrackState(App* app);

    void init() override;
    void terminate() override;
    void update() override;
    void handleEvent() override;
    void draw() override;
private:
    App*      app;
    long long messageReceiverID;

    void onMessage(core::Message message);
};