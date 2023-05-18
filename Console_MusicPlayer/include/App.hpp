#pragma once
#include "State/MenuState.hpp"
#include "core/Timer.hpp"  
#include "core/StateMachine.hpp"
#include "core/MessageBus.hpp"
#include "core/ScrollableList.hpp"

class App
{
public:
	struct LoadingScreenStyle
	{
		core::Color background;
		core::Color title;
		core::Color loadingText;
		core::Color loadingTextAnim;
	};

	/** Use this to set different themes. */
	struct Style
	{
		core::Color                 fgcolor; //< default color if nothing is specified
		core::Color                 bgcolor; //< default color if nothing is specified
		LoadingScreenStyle          loadingScreen;
		core::ScrollableList::Style scrollableList;
		MenuState::Style            menu;
	};

	core::MessageBus      messageBus;
	bool                  isRunning;
	core::StateMachine    stateMachine;
	MenuState             menuState;
	std::vector<fs::path> musicDirs;
	fs::path              currPlaylist;
	Style                 style;

	explicit App();
	explicit App(const App& other) = delete;
	void mainLoop();
	void handleEvents();
	void onMessage(core::Message message);
private:
	core::Timer drawTimer;
	void terminate();
};