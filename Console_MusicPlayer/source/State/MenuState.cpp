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

MenuState::MenuState(App* app) :
	app(app),
	playlistList(),
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
	style = app->style.menu;

	if (firstInit) {
		app->messageBus.add(std::bind(&MenuState::onMessage, this, std::placeholders::_1));
		// This may not be called in the constructor, because App::messagebus is at this time not initialized.
		// Well, it could be if in the class declaration it is above MenuState declared, but this is ugly.
		// Note: I don't want to delete myself as as an receiver in termiante(), because I still want to
		// receive messages, so I check for 'firstInit'.
	}

	// Set selected:
	// Do not do this in the constructor, because in App::App I need to check if the directory exists.
	std::map<std::wstring, std::wstring> config = core::getConfig("data/config.dat");
	if (config.count(L"defaultPlaylist") == 0) {
		core::log("Error: defaultPlaylist not found in config.dat");
		__debugbreak();
	}
	size_t selectedPlaylist = std::stoi(config[L"defaultPlaylist"]);
	core::ScrollableList::Options scrollListFlags = (core::ScrollableList::Options)(
		(int)core::ScrollableList::SelectionMode | 
		//(int)core::ScrollableList::ArrowInput | // I need this keys for SmallMusicPlayer
		(int)core::ScrollableList::DrawFullX); // When this is set then use ScrollableList::getPosX()

	playlistList.init(scrollListFlags, app->style.scrollableList, "playlists", 20, 60, selectedPlaylist);
	musicList.init(scrollListFlags, app->style.scrollableList, "tracks", 20);
	directoryList.init(scrollListFlags, app->style.scrollableList, "directories", 20);

	initMusicList();
	initDirectories();
	int musicListHalfDrawSize = round(musicList.getDrawSize() / 2.f);
	int musicPlayerDrawSize = musicListHalfDrawSize;
	smallMusicPlayer.init(app->musicDirs, musicListHalfDrawSize, getMusicPlayerPosX(), app->style.smallMusicPlayer);
	smallMusicPlayer.setDrawKeyInfo(drawKeyInfo);

	firstInit = false;
}

int MenuState::getMusicPlayerPosX()
{
	int musicListHalfDrawSize = round(musicList.getDrawSize() / 2.f);
	int musicPlayerDrawSize = musicListHalfDrawSize;
	return (musicListHalfDrawSize + musicList.getPosX()) - round(musicPlayerDrawSize / 2.f);
}

void MenuState::initMusicList()
{
	// Takes a lot of time when it has to load many music files (>300)

	for (auto& musicDir : app->musicDirs) {
		for (auto& it : fs::recursive_directory_iterator(musicDir)) {
			if (!core::isSupportedAudioFile(it.path())) {
				continue;
			}

			// Open music to get its metadata:
			Mix_Music* music = Mix_LoadMUS(it.path().u8string().c_str());
			if (!music) {
				core::log("Failed to load music! SDL_mixer Error: " + std::string(Mix_GetError()));
				//__debugbreak();
				return;
			}

			std::string filenameStem = it.path().stem().u8string();
			std::string sdlTitle = Mix_GetMusicTitle(music);
			std::string title = strcmp(Mix_GetMusicTitle(music), "") == 0 ? filenameStem : Mix_GetMusicTitle(music); // SDL2 does not return filename as mentioned, so I do it manually.
			core::Time duration = core::Time(core::Seconds((int)Mix_MusicDuration(music)));
			musicList.push_back(title);
			Mix_FreeMusic(music);
		}
	}
}

void MenuState::initDirectories()
{
	for (auto& musicDir : app->musicDirs) {
		directoryList.push_back(musicDir.u8string());
	}
}

void MenuState::terminate()
{
	// Well, I need the selected playlist in PlayState..
	musicList.terminate();
	playlistList.terminate();
	directoryList.terminate();
	smallMusicPlayer.terminate();
}

void MenuState::update()
{
	// Search for playlists:
	// I'm doing this regularly, so that user can add playlists while program is running.
	if (selected == Option::Playlists) {
		// ...playlist option is selected, but we must not be inside of it.
		playlistList.clear();
		playlistList.push_back("all"); // virtual playlist (plays all music)
		for (auto& it : fs::directory_iterator("data")) {
			if (it.is_regular_file() && it.path().extension() == ".pl") {
				playlistList.push_back(it.path().stem().string());
			}
		}
		playlistList.update();

		if (isInsideOption() && playlistList.isTrappedOnTop()) {
			// ...user wants to scroll out of the list.
			hover = Option::Last; // we leave playlist option, but it is still selected and thus rendered.
			playlistList.loseFocus();
		}
	}

	if (selected == Option::AllMusic) {
		musicList.update();
		if (isInsideOption() && musicList.isTrappedOnTop()) {
			// ...user wants to scroll out of the list.
			hover = Option::Last;
			musicList.loseFocus();
		}
	}

	if (selected == Option::Directories) {
		directoryList.update();
		if (isInsideOption() && directoryList.isTrappedOnTop()) {
			// ...user wants to scroll out of the list.
			hover = Option::Last;
			directoryList.loseFocus();
		}
	}

	// Update music list selection:
	if (!smallMusicPlayer.isStopped() && selected != musicList.getSelectedIndex() != -1) {
		// ..small music player is running
		musicList.select(smallMusicPlayer.getCurrentMusicIndex());
	}

	// Update list border color:
	if (smallMusicPlayer.getPlaylist().isCurrentMusicStopped()) {
		musicList.style.border     = core::Color::Light_Red;
		playlistList.style.border  = core::Color::Light_Red;
		directoryList.style.border = core::Color::Light_Red;
	}
	else if (smallMusicPlayer.getPlaylist().isPaused()) {
		musicList.style.border     = core::Color::Light_Aqua;
		playlistList.style.border  = core::Color::Light_Aqua;
		directoryList.style.border = core::Color::Light_Aqua;
	}
	else if (smallMusicPlayer.getPlaylist().isPlaying()) {
		musicList.style.border     = core::Color::Light_Green;
		playlistList.style.border  = core::Color::Light_Green;
		directoryList.style.border = core::Color::Light_Green;
	}

	smallMusicPlayer.update();
}

void MenuState::handleEvent()
{
	if (core::inputDevice::isKeyPressed(VK_RETURN)) {
		if (!isInsideOption()) {
			selected = hover;
			// Jump into the option:
			hover = None;
			if (selected == Option::AllMusic) {
				musicList.gainFocus();
				musicList.scrollToTop();
			}
			else if (selected == Option::Playlists) {
				playlistList.gainFocus();
				playlistList.scrollToTop();
			}
			else if (selected == Option::Directories) {
				directoryList.gainFocus();
				directoryList.scrollToTop();
			}
		}
		else if (selected == Option::AllMusic) {
			musicList.selectHoveredItem();
			smallMusicPlayer.play(musicList.getSelectedIndex());
		}
		else if (selected == Option::Playlists) {
			playlistList.selectHoveredItem();
			// Play playlist:
			std::string* userData = new std::string("data/" + playlistList.getSelected() + ".pl");
			app->messageBus.send(Message::MenuState_EnteredPlaylist, userData);
			// Update config:
			std::map<std::wstring, std::wstring> config = core::getConfig("data/config.dat");
			config[L"defaultPlaylist"] = std::to_wstring(playlistList.getSelectedIndex());
			core::setConfig("data/config.dat", config);
		}
		else if (selected == Option::Directories) {
			directoryList.selectHoveredItem();
		}
		else __debugbreak();

		// TODO: app->messageBus.send(Message::MenuState_SelectedPlaylistEditor);
	}

	if (core::inputDevice::isKeyPressed(VK_F12, true)) {
		core::inputDevice::lock(!core::inputDevice::isLocked());
	}

	if (core::inputDevice::isKeyPressed('K')) {
		drawKeyInfo = !drawKeyInfo;
		smallMusicPlayer.setDrawKeyInfo(drawKeyInfo);
	}

	if (core::inputDevice::isKeyPressed('O')) {
		hover = Option::First;
		if (selected == Option::Playlists) playlistList.loseFocus();
		else if (selected == Option::AllMusic) musicList.loseFocus();
		else if (selected == Option::Directories) directoryList.loseFocus();
	}

	if (!isInsideOption())
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
					hover = None;
					if (selected == Option::AllMusic) musicList.gainFocus();
					else if (selected == Option::Playlists) playlistList.gainFocus();
					else if (selected == Option::Directories) directoryList.gainFocus();
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
		if (core::inputDevice::isKeyPressed(VK_UP)) {
			moveUp();
		}
		if (core::inputDevice::isKeyPressed(VK_DOWN)) {
			moveDown();
		}
	}
	else if (selected == Option::AllMusic) {
		musicList.handleEvent();
	}
	else if (selected == Option::Playlists) {
		playlistList.handleEvent();
	}
	else if (selected == Option::Directories) {
		directoryList.handleEvent();
	}

	smallMusicPlayer.handleEvent();
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
	const core::Playlist& playlist = smallMusicPlayer.getPlaylist();
	// Exit info:
	std::string exitInfo = drawKeyInfo ? " <[ESC]"s : " <"; // If you use core::uc::leftwardsArrow, then note that std::string cannot properly handle unicode and its length is wrong.
	// List status:
	std::string trackNum = playlist.size() > 0 ? std::to_string(playlist.getCurrentMusicNumber()) : "-";
	//if (playlist.getCurrentMusicNumber() > playlist.size()) trackNum = std::to_string(playlist.size());
	trackNum += " / " + (playlist.size() > 0 ? std::to_string(playlist.size()) : "-");
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
	if (!playlist.isCurrentMusicStopped()) {
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
	if (!playlist.isCurrentMusicStopped()) {
		// Track name:
		const int maxNameSize = core::console::getCharCount().x - 4; // -4 because I want some padding
		std::string trackName = smallMusicPlayer.getPlaylist().current().title.substr(0, maxNameSize);
		if (trackName.length() < smallMusicPlayer.getPlaylist().current().title.length()) {
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
		musicList.draw();
	}
	else if (selected == Option::Playlists) {
		playlistList.draw();
	}
	else if (selected == Option::Directories) {
		directoryList.draw();
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
		if (playlist.isPlaying()) playbackStatus = core::Text(" playing", style.statusOn);
		else if (playlist.isPaused()) playbackStatus = core::Text(" paused", core::Color::Aqua);
		core::Text playbackKey = core::Text(drawKeyInfo ? " [p]" : "", core::Color::Gray);
		// shuffle:
		core::Text shuffleStatus = core::Text("shuffle ", playlist.isShuffled() ? style.statusOn : style.statusOff);
		core::Text shuffleKey = core::Text(drawKeyInfo ? "[r] " : "", core::Color::Gray);
		int shufflePos = core::console::getCharCount().x - (playbackStatus.str.length() + shuffleStatus.str.length() + playbackKey.str.length() + shuffleKey.str.length());
		// volume:
		core::Text volume = core::Text(" vol "s + std::to_string((int)playlist.getVolume()) + "%", core::Color::White);
		core::Text volumeKey = core::Text(drawKeyInfo ? " [+/-]" : "", core::Color::Gray);
		// repeat:
		core::Text repeatStatus = core::Text("repeat off ", style.statusOff);
		core::Text repeatKey = core::Text(drawKeyInfo ? "[l] " : "", core::Color::Gray);
		if (smallMusicPlayer.replay == core::SmallMusicPlayer::Replay::One) repeatStatus = core::Text("repeat track ", style.statusOn);
		else if (smallMusicPlayer.replay == core::SmallMusicPlayer::Replay::All) repeatStatus = core::Text("repeat list ", style.statusOn);
		int repeatPos = core::console::getCharCount().x - (volume.str.length() + repeatStatus.str.length() + volumeKey.str.length() + repeatKey.str.length() + smallMusicPlayer.volumeReport.str.length());
		
		std::cout << playbackStatus << playbackKey << std::string(shufflePos - 1, ' ') << shuffleKey << shuffleStatus << core::endl()
			<< volume << smallMusicPlayer.volumeReport << volumeKey << std::string(repeatPos - 1, ' ') << repeatKey << repeatStatus << core::endl(2);
	}
	
	///////////////////////////////////////////////////////////////////////////////
	// Duration
	///////////////////////////////////////////////////////////////////////////////
	// BUG: progressBarSize is not accurate - there is still space left after track finishes.
	{
		core::Time currPlaytime = playlist.isCurrentMusicStopped() ? core::Time() : playlist.getCurrentMusicElapsedTime();
		core::Time currPlaytimeMax = playlist.isCurrentMusicStopped() ? core::Time() : playlist.getCurrentMusicDuration();
		core::Text skipReport = smallMusicPlayer.skipReport;
		std::string currDuration = playlist.size() == 0 ? "-" : getTimeStr(currPlaytime, currPlaytimeMax);
		std::string duration = playlist.size() == 0 ? "-" : getTimeStr(currPlaytimeMax);
		std::string durationInfo = currDuration + skipReport.str + " / " + duration;
		std::string durationSkipForwardKeyInfo = drawKeyInfo ? "["s + core::uc::leftwardsArrow + "] " : "";
		std::string durationSkipBackwardKeyInfo = drawKeyInfo ? " ["s + core::uc::rightwardsArrow + "]" : "";
		int durationKeyInfoLength = durationSkipForwardKeyInfo.length() > 0 ? 8 : 0; // [<] ... [>]; note length is wrong because of unicode characters
		drawPos = core::console::getCharCount().x / 2.f - (durationInfo.length() + durationKeyInfoLength) / 2.f;
		skipReport.bgcolor = core::Color::White; // TODO set it in constructor
		core::console::setBgColor(core::Color::Gray);

		float progressBarFactor = currPlaytime.asSeconds() / (currPlaytimeMax == 0s ? 1 : playlist.getCurrentMusicDuration().asSeconds()); // 0..duration
		int progressBarSize = core::console::getCharCount().x * progressBarFactor; // 0..consoleCharCountX

		std::string spaceBefore(drawPos, ' ');
		std::string spaceAfter; // can not be initialized here because of getCursorPos()
		coutWithProgressBar(progressBarSize, spaceBefore, style.durationText, style.durationProgressBarText, style.durationProgressBar);
		coutWithProgressBar(progressBarSize, durationSkipForwardKeyInfo, core::Color::White, style.durationProgressBarText, style.durationProgressBar);
		coutWithProgressBar(progressBarSize, currDuration, style.durationText, style.durationProgressBarText, style.durationProgressBar);
		coutWithProgressBar(progressBarSize, skipReport.str, skipReport.fgcolor, skipReport.bgcolor, style.durationProgressBar);
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
		musicList.onConsoleResize();
		playlistList.onConsoleResize();
		directoryList.onConsoleResize();
		smallMusicPlayer.setPosX(getMusicPlayerPosX());
	}
}

bool MenuState::isInsideOption()
{
	return hover == Option::None;
}

bool MenuState::isInsideOption(Option option)
{
	return isInsideOption() && option == selected;
}