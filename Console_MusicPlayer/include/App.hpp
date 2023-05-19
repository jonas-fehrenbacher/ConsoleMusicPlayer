#pragma once
#include "State/TrackState.hpp"
#include "State/PlaylistState.hpp"
#include "State/DirectoryState.hpp"
#include "Title.hpp"
#include "NavBar.hpp"
#include "PlayStatus.hpp"
#include "Footer.hpp"
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

	core::MessageBus      messageBus;
	bool                  isRunning;
	core::State*          activeState;
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

	explicit App();
	explicit App(const App& other) = delete;
	void mainLoop();
	void update();
	void draw();
	void handleEvents();
	void onMessage(core::Message message);
private:
	core::Timer drawTimer;
	void terminate();
};