#pragma once
#include "State/MenuState.hpp"
#include "State/PlayState.hpp"
#include "State/PlaylistEditorState.hpp"
#include "core/Timer.hpp"  
#include "core/StateMachine.hpp"
#include "core/MessageBus.hpp"

class App
{
public:
	bool                  isRunning;
	core::StateMachine    stateMachine;
	MenuState             menuState;
	PlayState             playState;
	PlaylistEditorState   playlistEditorState;
	core::MessageBus      messageBus;
	std::vector<fs::path> musicDirs;
	fs::path              currPlaylist;

	explicit App();
	explicit App(const App& other) = delete;
	void mainLoop();
	void handleEvents();
	void onMessage(core::Message message);
private:
	core::Timer drawTimer;
	void terminate();
};