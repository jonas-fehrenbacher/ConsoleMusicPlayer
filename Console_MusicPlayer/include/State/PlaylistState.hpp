#pragma once

#include "core/DrawableList.hpp"
#include "core/StateMachine.hpp"
#include "core/MessageBus.hpp"

class App;
namespace core {
    class MusicPlayer;
}

class PlaylistState : public core::State
{
public:
    enum class State
    {
        Playlist,
        PlaylistList
        // TODO Both
    };

    PlaylistState(App* app);

    void init() override;
    void terminate() override;
    void update() override;
    void handleEvent() override;
    void draw() override;
private:
    App*                 app;
    core::DrawableList   playlistList;
    State                state;
    long long            messageReceiverID;

    bool isTrappedOnTop();
    void scrollToTop();
    void onMessage(core::Message message);
};