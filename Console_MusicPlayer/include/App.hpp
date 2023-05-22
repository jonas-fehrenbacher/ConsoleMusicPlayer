#pragma once
#include "State/TrackState.hpp"
#include "State/PlaylistState.hpp"
#include "State/DirectoryState.hpp"
#include "Title.hpp"
#include "NavBar.hpp"
#include "PlayStatus.hpp"
#include "Footer.hpp"
#include "Keymap.hpp"
#include "core/Timer.hpp"  
#include "core/StateMachine.hpp"
#include "core/MessageBus.hpp"
#include "core/DrawableList.hpp"
#include "core/MusicPlayer.hpp"

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
		core::FullColor             default; //< default color if nothing is specified
		LoadingScreenStyle          loadingScreen;
		core::DrawableList::Style   drawableList;
		Title::Style                title;
		NavBar::Style               navBar;
		PlayStatus::Style           playStatus;
		Footer::Style               footer;
	};

	const fs::path        configFilePath;
	core::MessageBus      messageBus;
	bool                  isRunning;
	core::ActiveState     activeState;
	TrackState            trackState;
	PlaylistState         playlistState;
	DirectoryState        directoryState;

	std::vector<fs::path> musicDirs;
	fs::path              currPlaylist;
	Style                 style;
	bool                  isDrawKeyInfo;
	core::MusicPlayer     musicPlayer;
	Title                 title;
	NavBar                navBar;
	PlayStatus            playStatus;
	Footer                footer;
	Keymap                keymap;

	explicit App();
	explicit App(const App& other) = delete;
	void mainLoop();
	void update();
	void draw();
	void handleEvents();
	void onMessage(core::Message message);
	core::Time getFrametime();
private:
	core::Timer drawTimer;
	core::Time  frametime;
	void terminate();
};