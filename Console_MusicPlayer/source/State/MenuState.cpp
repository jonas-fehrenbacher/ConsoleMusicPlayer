#include "State/MenuState.hpp"
#include "Messages.hpp"
#include "App.hpp"
#include "core/SmallTools.hpp"
#include "core/InputDevice.hpp"
#include "core/Console.hpp"
#include "core/ScrollableList.hpp"
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <iostream>
#include <filesystem>
#include <cassert>

enum Column
{
	COLUMN_NAME
};

MenuState::MenuState(App* app) :
	app(app),
	options( { "Tracks", "Playlists", "Directories" }),
	selected(Option::AllMusic),
	hover(Option::None),
	drawKeyInfo(true),
	firstInit(true),
	style()
{
	
}

void MenuState::init()
{
	if (firstInit) {
		app->messageBus.add(std::bind(&MenuState::onMessage, this, std::placeholders::_1));
		// This may not be called in the constructor, because App::messagebus is at this time not initialized.
		// Well, it could be if in the class declaration it is above MenuState declared, but this is ugly.
		// Note: I don't want to delete myself as as an receiver in termiante(), because I still want to
		// receive messages, so I check for 'firstInit'.
	}

	///////////////////////////////////////////////////////////////////////////////
	// Get config
	///////////////////////////////////////////////////////////////////////////////
	// Do not do this in the constructor, because in App::App I need to check if the directory exists.
	std::map<std::wstring, std::wstring> config = core::getConfig("data/config.dat");
	if (config.count(L"defaultPlaylist") == 0) {
		core::log("Error: defaultPlaylist not found in config.dat");
		__debugbreak();
	}

	///////////////////////////////////////////////////////////////////////////////
	// Init music player
	///////////////////////////////////////////////////////////////////////////////
	core::Time musicPlayer_sleepTime = 0s;
	if (config[L"totalRuntime"] != L"nolimit") {
		musicPlayer_sleepTime = core::Seconds(std::stoi(config[L"totalRuntime"]));
	}
	int musicPlayer_options =
		(config[L"isPlaylistShuffled"] == L"true" ? core::MusicPlayer::Shuffle : 0) |
		(config[L"playlistLoop"] == L"none" ? 0 : (config[L"playlistLoop"] == L"one" ? core::MusicPlayer::LoopOne : core::MusicPlayer::LoopAll)) |
		core::MusicPlayer::FadeOut;
	musicPlayer.init(app->musicDirs, app->style.scrollableList, musicPlayer_options, musicPlayer_sleepTime);

	firstInit = false;
	style = app->style.menu;

	playlistState.init(app, &musicPlayer);
	directoryState.init(app);
}

// deprecated:
void initMusicList()
{
#if 0
	// Takes a lot of time when it has to load many music files (>300)

	for (auto& musicDir : app->musicDirs) {
		for (auto it : fs::recursive_directory_iterator(musicDir)) {
			if (!core::isSupportedAudioFile(it.path())) {
				if (core::isAudioFile(it.path()))
					core::log("Warning: Music format is not supported: "s + fs::path(it.path()).make_preferred().string());
				continue;
			}

			// Open music to get its metadata:
			Mix_Music* music = Mix_LoadMUS(it.path().u8string().c_str());
			if (!music) {
				core::log("Failed to load music! SDL_mixer Error: " + std::string(Mix_GetError()));
				//__debugbreak();
				return;
			}

			std::string filenameStem = "";
			try {
				filenameStem = it.path().stem().u8string(); // u8string is important; otherwise I get here exceptions and wstring_convert fails.
			}
			catch (...) {
				filenameStem = it.path().stem().u8string(); // some utf8 filenames cause an exception.
				// core::toWStr(filenameStem) returns correct characters - even by 이루마 which causes trouble.
			}
			std::string sdlTitle = Mix_GetMusicTitle(music);
			std::string title = strcmp(Mix_GetMusicTitle(music), "") == 0 ? filenameStem : Mix_GetMusicTitle(music); // SDL2 does not return filename as mentioned, so I do it manually.
			core::Time duration = core::Time(core::Seconds((int)Mix_MusicDuration(music)));
			musicList.push_back({ title, core::getTimeStr(duration) });
			Mix_FreeMusic(music);
		}
	}

	// Calculate everything new (important):
	musicList.onConsoleResize();
#endif
}

void MenuState::terminate()
{
	// Well, I need the selected playlist in PlayState..
	playlistState.terminate();
	directoryState.terminate();
	musicPlayer.terminate();
}

void MenuState::update()
{
	///////////////////////////////////////////////////////////////////////////////
	// Update state
	///////////////////////////////////////////////////////////////////////////////
	if (selected == Option::AllMusic) {
		
	}
	else if (selected == Option::Playlists) {
		playlistState.update();
	}
	else if (selected == Option::Directories) {
		directoryState.update();
	}

	///////////////////////////////////////////////////////////////////////////////
	// Scrollable lists breakout to navigation bar
	///////////////////////////////////////////////////////////////////////////////
	if (!isInsideNavBar())
	{
		bool isTrappedOnTop = false;
		if (selected == Option::AllMusic && musicPlayer.isTrappedOnTop()) {
			isTrappedOnTop = true;
			musicPlayer.stopDrawableListEvents();
		}
		else if (selected == Option::Playlists && playlistState.isTrappedOnTop()) {
			isTrappedOnTop = true;
			playlistState.loseFocus();
		}
		else if (selected == Option::Directories && directoryState.isTrappedOnTop()) {
			isTrappedOnTop = true;
			directoryState.loseFocus();
		}
		if (isTrappedOnTop) {
			hover = Option::Last; // we leave list, but it is still rendered.
		}
	}

	musicPlayer.update();
}

void MenuState::handleEvent()
{
	musicPlayer.handleEvents();

	///////////////////////////////////////////////////////////////////////////////
	// Lock
	///////////////////////////////////////////////////////////////////////////////
	if (core::inputDevice::isKeyPressed(VK_F12, true)) {
		core::inputDevice::lock(!core::inputDevice::isLocked());
	}

	///////////////////////////////////////////////////////////////////////////////
	// Hide key
	///////////////////////////////////////////////////////////////////////////////
	if (core::inputDevice::isKeyPressed('K')) {
		drawKeyInfo = !drawKeyInfo;
		playlistState.setDrawKeyInfo(drawKeyInfo);
	}

	///////////////////////////////////////////////////////////////////////////////
	// Jump to navigation bar
	///////////////////////////////////////////////////////////////////////////////
	if (core::inputDevice::isKeyPressed('O')) {
		hover = Option::First;
		if (selected == Option::Playlists) playlistState.loseFocus();
		else if (selected == Option::AllMusic) musicPlayer.stopDrawableListEvents();
		else if (selected == Option::Directories) directoryState.loseFocus();
	}

	///////////////////////////////////////////////////////////////////////////////
	// Navigation bar slection
	///////////////////////////////////////////////////////////////////////////////
	if (isInsideNavBar() && core::inputDevice::isKeyPressed(VK_RETURN))
	{
		selected = hover;
		// Jump into the option:
		hover = Option::None;

		if (selected == Option::AllMusic) {
			musicPlayer.resumeDrawableListEvents();
			musicPlayer.scrollDrawableListToTop();
			musicPlayer.setDrawnPlaylist(core::MusicPlayer::ALL_PLAYLIST_NAME);
		}
		else if (selected == Option::Playlists) {
			playlistState.start();
		}
		else if (selected == Option::Directories) {
			directoryState.update();
			directoryState.gainFocus();
			directoryState.scrollToTop();
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// Navigation bar selection movement
	///////////////////////////////////////////////////////////////////////////////
	if (isInsideNavBar())
	{
		// ..we are in the option selection

		// Number input:
		//for (int i = '0'; i <= '9'; ++i) {
		//	if (i > '0' && core::inputDevice::isKeyPressed(i)) {
		//		// Playlists are listed from number 1.
		//		hover = i - '0';
		//	}
		//}

		auto moveUp = [&]() {
			hover = (Option)(hover - 1);
			if (hover < 0) {
				if (selected != Option::None)
					hover = Option::First;
				else hover = Option::Last;
			}
		};
		auto moveDown = [&]() {
			hover = (Option)(hover + 1);
			if (hover >= options.size()) {
				if (selected != Option::None) 
				{
					// Jump into the option:
					hover = Option::None;
					if (selected == Option::AllMusic) musicPlayer.resumeDrawableListEvents();
					else if (selected == Option::Playlists) playlistState.gainFocus();
					else if (selected == Option::Directories) directoryState.gainFocus();
				}
				else hover = Option::First;
			}
		};

		std::vector<core::inputDevice::MouseWheelScroll> mouseWheelScrollEvents = core::inputDevice::getMouseWheelScrollEvents();
		for (auto& mouseWheelScrollEvent : mouseWheelScrollEvents) {
			if (mouseWheelScrollEvent == core::inputDevice::MouseWheelScroll::Up) {
				moveUp();
			}
			else moveDown();
		}
		// up / down keys are reserved for music player:
		//if (core::inputDevice::isKeyPressed(VK_UP)) {
		//	moveUp();
		//}
		//if (core::inputDevice::isKeyPressed(VK_DOWN)) {
		//	moveDown();
		//}
	}

	///////////////////////////////////////////////////////////////////////////////
	// Handle state events
	///////////////////////////////////////////////////////////////////////////////
	if (selected == Option::AllMusic) {
		
	}
	else if (selected == Option::Playlists) {
		playlistState.handleEvent();
	}
	else if (selected == Option::Directories) {
		directoryState.handleEvent();
	}
}

intern void coutWithProgressBar(int& progressBarSize, std::string text, core::Color textColor, core::Color progressBarTextColor, core::Color progressBarColor)
{
	std::cout << core::Text(progressBarSize > text.size() ? text : text.substr(0, progressBarSize), progressBarSize > 0 ? progressBarTextColor : textColor, progressBarSize > 0 ? progressBarColor : core::Color::None)
		<< core::Text(progressBarSize > text.size() ? "" : text.substr(progressBarSize, text.size() - progressBarSize), textColor, core::Color::None);
	progressBarSize -= text.size();
}

void MenuState::draw()
{
	// Set background color:
	core::console::setFgColor(style.item); // Maybe app should do this...

	///////////////////////////////////////////////////////////////////////////////
	// Draw first line of the header
	///////////////////////////////////////////////////////////////////////////////
	// Exit info:
	std::string exitInfo = drawKeyInfo ? " <[ESC]"s : " <"; // If you use core::uc::leftwardsArrow, then note that std::string cannot properly handle unicode and its length is wrong.
	// List status:
	std::string trackNum = musicPlayer.getActivePlaylistSize() > 0 ? std::to_string(musicPlayer.getActivePlaylistCurrentTrackNumber()) : "-";
	//if (musicPlayer.getActivePlaylistCurrentTrackNumber() > musicPlayer.getActivePlaylistSize()) trackNum = std::to_string(musicPlayer.getActivePlaylistSize());
	trackNum += " / " + (musicPlayer.getActivePlaylistSize() > 0 ? std::to_string(musicPlayer.getActivePlaylistSize()) : "-");
	// skip key info:
	std::string skipForwardKeyInfo = drawKeyInfo ? "["s + core::uc::downwardsArrow + "] " : "";
	std::string skipBackwardKeyInfo = drawKeyInfo ? " ["s + core::uc::upwardsArrow + "]" : "";
	// Set draw position:
	int trackNumKeyInfoLength = skipForwardKeyInfo.length() > 0 ? 8 : 0; // [^] ... [v]; note length is wrong because of unicode characters
	int trackNumberDrawPos = core::console::getCharCount().x / 2.f - (trackNum.length() + trackNumKeyInfoLength) / 2.f;
	trackNumberDrawPos -= exitInfo.length();
	// Draw:
	core::console::setBgColor(core::Color::Bright_White);
	std::cout << core::Text(exitInfo, core::Color::White);
	if (!musicPlayer.isStopped()) {
		// ..draw track number if music is playing.
		std::cout << core::Text(std::string(trackNumberDrawPos, ' '))
			<< core::Text(skipForwardKeyInfo, core::Color::White)
			<< core::Text(trackNum, core::Color::Black)
			<< core::Text(skipBackwardKeyInfo, core::Color::White);
	}
	// Lock events:
	core::Text inputInfo(drawKeyInfo ? "[F12] " : "", core::Color::White);
	if (core::inputDevice::isLocked())
		inputInfo.str += "Locked Input";
	if (!core::inputDevice::isLocked())
		inputInfo.str += "Free Input";
	int drawPos = (core::console::getCharCount().x - core::console::getCursorPos().x) - inputInfo.str.length();
	std::cout << core::Text(std::string(drawPos - 1, ' '))
		<< inputInfo << core::endl();

	///////////////////////////////////////////////////////////////////////////////
	// Draw track or title of the header
	///////////////////////////////////////////////////////////////////////////////
	if (!musicPlayer.isStopped()) {
		// Track name:
		const int maxNameSize = core::console::getCharCount().x - 4; // -4 because I want some padding
		std::string trackName = musicPlayer.getPlayingMusicInfo().title.substr(0, maxNameSize);
		if (trackName.length() < musicPlayer.getPlayingMusicInfo().title.length()) {
			trackName.replace(trackName.end() - 2, trackName.end(), "..");
		}
		drawPos = core::console::getCharCount().x / 2.f - trackName.length() / 2.f;
		trackName.insert(0, drawPos, ' ');
		std::cout << core::endl() << trackName << core::endl(2);
	}
	else {
		// Title:
		std::string title = "Console Music Player "s + core::uc::eighthNote;
		drawPos = core::console::getCharCount().x / 2.f - (title.length() - 2) / 2.f; // -2 because std::string cannot handle unicode characters - the length is to much.
		title.insert(0, drawPos, ' ');
		// Notice:
		std::string notice = "(Select and press enter)";
		drawPos = core::console::getCharCount().x / 2.f - notice.length() / 2.f;
		notice.insert(0, drawPos, ' ');
		// Draw:
		std::cout << core::Text(title, core::Color::Black) << core::endl()
			<< core::Text(drawKeyInfo ? notice : "", core::Color::White) << core::endl()
			<< core::endl(core::Color::Bright_White);
	}

	///////////////////////////////////////////////////////////////////////////////
	// Draw navigation bar of the header
	///////////////////////////////////////////////////////////////////////////////

	// options:
	core::console::setBgColor(core::Color::Bright_White);
	std::string optionsKeyInfo = drawKeyInfo ? "[O] " : "";
	int optionSpaceInbetween = 3;
	int optionsSize = 0;
	for (int i = 0; i < options.size(); ++i) optionsSize += options[i].size();
	optionsSize += (options.size() - 1) * optionSpaceInbetween; // space inbetween
	optionsSize += 2; // at the beginning and end is " " for highlighting
	optionsSize += optionsKeyInfo.length();
	drawPos = core::console::getCharCount().x / 2.f - optionsSize / 2.f;
	std::cout << std::string(drawPos, ' ');

	for (int i = 0; i < options.size(); ++i) {
		core::Text next = core::Text(" " + options[i] + " ");
		if (i == hover) {
			next.fgcolor = style.hover; 
			next.bgcolor = core::Color::White;
		}
		else if (i == selected) {
			next.fgcolor = style.selected;
			next.bgcolor = core::Color::Gray;
		}
		else {
			next.fgcolor = style.item;
			next.bgcolor = core::Color::None;
		}
		std::cout 
			<< core::Text(i == 0 ? optionsKeyInfo : "", core::Color::Gray)
			<< next
			<< std::string(i == options.size() - 1? 0 : optionSpaceInbetween - 2, ' ');
	}
	std::cout << core::endl();

	///////////////////////////////////////////////////////////////////////////////
	// Draw scrollable list
	///////////////////////////////////////////////////////////////////////////////

	core::console::setBgColor(app->style.bgcolor);
	std::cout << core::endl();
	if (selected == Option::AllMusic) {
		musicPlayer.draw();
	}
	else if (selected == Option::Playlists) {
		playlistState.draw();
	}
	else if (selected == Option::Directories) {
		directoryState.draw();
	}

	///////////////////////////////////////////////////////////////////////////////
	// Track status
	///////////////////////////////////////////////////////////////////////////////
	{
		int lineCount = core::console::getCharCount().y - 4;
		for (int i = core::console::getCursorPos().y; i < lineCount - 1; ++i) {
			std::cout << core::endl();
		}

		// playback:
		core::Text playbackStatus = core::Text(" playback is stopped", style.statusOff);
		if (musicPlayer.isPlaying()) playbackStatus = core::Text(" playing", style.statusOn);
		else if (musicPlayer.isPaused()) playbackStatus = core::Text(" paused", core::Color::Aqua);
		core::Text playbackKey = core::Text(drawKeyInfo ? " [p]" : "", core::Color::Gray);
		// shuffle:
		core::Text shuffleStatus = core::Text("shuffle ", musicPlayer.isShuffled() ? style.statusOn : style.statusOff);
		core::Text shuffleKey = core::Text(drawKeyInfo ? "[r] " : "", core::Color::Gray);
		int shufflePos = core::console::getCharCount().x - (playbackStatus.str.length() + shuffleStatus.str.length() + playbackKey.str.length() + shuffleKey.str.length());
		// volume:
		core::Text volume = core::Text(" vol "s + std::to_string((int)musicPlayer.getVolume()) + "%", core::Color::White);
		core::Text volumeKey = core::Text(drawKeyInfo ? " [+/-]" : "", core::Color::Gray);
		// repeat:
		core::Text repeatStatus = core::Text("repeat off ", style.statusOff);
		core::Text repeatKey = core::Text(drawKeyInfo ? "[l] " : "", core::Color::Gray);
		if (musicPlayer.getReplayStatus() == core::MusicPlayer::Replay::One) repeatStatus = core::Text("repeat track ", style.statusOn);
		else if (musicPlayer.getReplayStatus() == core::MusicPlayer::Replay::All) repeatStatus = core::Text("repeat list ", style.statusOn);
		int repeatPos = core::console::getCharCount().x - (volume.str.length() + repeatStatus.str.length() + volumeKey.str.length() + repeatKey.str.length() + musicPlayer.volumeReport.str.length());
		
		std::cout << playbackStatus << playbackKey << std::string(shufflePos - 1, ' ') << shuffleKey << shuffleStatus << core::endl()
			<< volume << musicPlayer.volumeReport << volumeKey << std::string(repeatPos - 1, ' ') << repeatKey << repeatStatus << core::endl(2);
	}
	/*
		if (playlist.getCurrentMusicLoop())
			std::cout << " [loop]";
		else
		{
			if (playlist.current().playCount == 1)
				std::cout << " [" << core::Text("1", core::Color::Bright_White) << " time]";
			else std::cout << " [" << core::Text(std::to_string(playlist.current().playCount), core::Color::Bright_White) << " times]";
		}
		std::cout << core::Text(drawKeyInfo ? " [W/S; (L)oop]" : "", core::Color::Gray) << core::endl();
	*/
	
	///////////////////////////////////////////////////////////////////////////////
	// Duration
	///////////////////////////////////////////////////////////////////////////////
	// BUG: progressBarSize is not accurate - there is still space left after track finishes.
	{
		core::Time currPlaytime = musicPlayer.isStopped() ? core::Time() : musicPlayer.getPlayingMusicElapsedTime();
		core::Time currPlaytimeMax = musicPlayer.isStopped() ? core::Time() : musicPlayer.getPlayingMusicInfo().duration;
		std::string currDuration = musicPlayer.getActivePlaylistSize() == 0 ? "-" : getTimeStr(currPlaytime, currPlaytimeMax);
		std::string duration = musicPlayer.getActivePlaylistSize() == 0 ? "-" : getTimeStr(currPlaytimeMax);
		std::string durationInfo = currDuration + musicPlayer.skipReport.str + " / " + duration;
		std::string durationSkipForwardKeyInfo = drawKeyInfo ? "["s + core::uc::leftwardsArrow + "] " : "";
		std::string durationSkipBackwardKeyInfo = drawKeyInfo ? " ["s + core::uc::rightwardsArrow + "]" : "";
		int durationKeyInfoLength = durationSkipForwardKeyInfo.length() > 0 ? 8 : 0; // [<] ... [>]; note length is wrong because of unicode characters
		drawPos = core::console::getCharCount().x / 2.f - (durationInfo.length() + durationKeyInfoLength) / 2.f;
		//musicPlayer.skipReport.bgcolor = core::Color::White; // TODO set it in constructor
		core::console::setBgColor(core::Color::Gray);

		float progressBarFactor = currPlaytime.asSeconds() / (currPlaytimeMax == 0s ? 1 : musicPlayer.getPlayingMusicInfo().duration.asSeconds()); // 0..duration
		int progressBarSize = core::console::getCharCount().x * progressBarFactor; // 0..consoleCharCountX

		std::string spaceBefore(drawPos, ' ');
		std::string spaceAfter; // can not be initialized here because of getCursorPos()
		coutWithProgressBar(progressBarSize, spaceBefore, style.durationText, style.durationProgressBarText, style.durationProgressBar);
		coutWithProgressBar(progressBarSize, durationSkipForwardKeyInfo, core::Color::White, style.durationProgressBarText, style.durationProgressBar);
		coutWithProgressBar(progressBarSize, currDuration, style.durationText, style.durationProgressBarText, style.durationProgressBar);
		coutWithProgressBar(progressBarSize, musicPlayer.skipReport.str, musicPlayer.skipReport.fgcolor, musicPlayer.skipReport.bgcolor, style.durationProgressBar);
		coutWithProgressBar(progressBarSize, " / ", style.durationText, style.durationProgressBarText, style.durationProgressBar);
		coutWithProgressBar(progressBarSize, duration, style.durationText, style.durationProgressBarText, style.durationProgressBar);
		coutWithProgressBar(progressBarSize, durationSkipBackwardKeyInfo, core::Color::White, style.durationProgressBarText, style.durationProgressBar);
		spaceAfter = std::string(core::console::getCharCount().x - (core::console::getCursorPos().x + 1), ' ');
		coutWithProgressBar(progressBarSize, spaceAfter, style.durationText, style.durationProgressBarText, style.durationProgressBar);
		std::cout << core::endl();
	}

	///////////////////////////////////////////////////////////////////////////////
	// Draw footer
	///////////////////////////////////////////////////////////////////////////////
	{
		int lineCount = core::console::getCharCount().y;
		for (int i = core::console::getCursorPos().y; i < lineCount - 1; ++i) {
			std::cout << core::endl();
		}
		core::console::setBgColor(core::Color::White);
		std::cout << " " << core::Text(" k ", core::Color::Bright_White, core::Color::Aqua)
			<< core::Text(drawKeyInfo ? " hide keys" : " show keys", core::Color::Gray);
		std::cout << core::Text(std::string(core::console::getCharCount().x - core::console::getCursorPos().x, ' '), core::Color::Gray);
	}
}

void MenuState::onMessage(core::Message message)
{
	if (message.id == core::MessageID::CONSOLE_RESIZE) {
		musicPlayer.onConsoleResize();
		playlistState.onConsoleResize();
		directoryState.onConsoleResize();
	}
}

bool MenuState::isInsideNavBar()
{
	return hover != Option::None;
}