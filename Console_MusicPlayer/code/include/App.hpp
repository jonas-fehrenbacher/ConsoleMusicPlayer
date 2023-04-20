#pragma once
#include "Timer/Timer.hpp"  
#include "State/StateMachine.hpp"
#include "State/MenuState.hpp"
#include "State/PlayState.hpp"
#include "State/PlaylistEditorState.hpp"
#include "Message/MessageBus.hpp"

class App
{
public:
	bool                isRunning;
	core::StateMachine  stateMachine;
	MenuState           menuState;
	PlayState           playState;
	PlaylistEditorState playlistEditorState;
	core::MessageBus    messageBus;

	explicit App();
	explicit App(const App& other) = delete;
	void mainLoop();
	void handleEvents();
	void onMessage(int message);
private:
	core::Timer drawTimer;
};