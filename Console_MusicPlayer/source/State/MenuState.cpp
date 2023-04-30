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

MenuState::MenuState(App* app) :
	app(app),
	playlistList(),
	options( { "All music", "Playlists", "Directories" }),
	selected(Option::AllMusic),
	hover(Option::None),
	drawKeyInfo(true)
{
	app->messageBus.add(std::bind(&MenuState::onMessage, this, std::placeholders::_1)); // May not be set in init() or it must be removed in terminate()
}

void MenuState::init()
{
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
		(int)core::ScrollableList::DrawCentered); // When this is set then use ScrollableList::getPosX()
	playlistList.init(scrollListFlags, 15, 60, selectedPlaylist);
	musicList.init(scrollListFlags);
	directoryList.init(scrollListFlags);

	initMusicList();
	initDirectories();
	int musicListHalfDrawSize = round(musicList.getDrawSize() / 2.f);
	int musicPlayerDrawSize = musicListHalfDrawSize;
	smallMusicPlayer.init(app->musicDirs, musicListHalfDrawSize, getMusicPlayerPosX());
	smallMusicPlayer.setDrawKeyInfo(drawKeyInfo);
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

void MenuState::draw()
{
	// Title:
	std::string title = "Console Music Player "s + core::uc::eighthNote;
	int drawPos = core::console::getCharCount().x / 2.f - (title.length() - 2) / 2.f; // -2 because std::string cannot handle unicode characters - the length is to much.
	title.insert(0, drawPos, ' ');
	// Notice:
	std::string notice = "(Select and press enter)";
	drawPos = core::console::getCharCount().x / 2.f - notice.length() / 2.f;
	notice.insert(0, drawPos, ' ');

	// Draw ESC:
	core::Color bgcolor = core::Color::Bright_White;
	std::cout << core::ColoredStr(drawKeyInfo ? " <[ESC]" : " <", core::Color::White, bgcolor);
	// Draw F12:
	core::ColoredStr inputInfo(drawKeyInfo ? "[F12] " : "", core::Color::White, core::Color::Bright_White);
	if (core::inputDevice::isLocked())
		inputInfo.str += "Locked Input";
	if (!core::inputDevice::isLocked())
		inputInfo.str += "Free Input";
	drawPos = (core::console::getCharCount().x - core::console::getCursorPos().x) - inputInfo.str.length();
	std::cout << core::ColoredStr(std::string(drawPos - 1, ' '), core::Color::White, core::Color::Bright_White)
		<< inputInfo << core::endl{ core::Color::Bright_White };
	// Draw rest of title bar:
	std::cout << core::ColoredStr(title, core::Color::Black, bgcolor) << core::endl{ bgcolor }
		<< core::ColoredStr(drawKeyInfo ? notice : "", core::Color::White, bgcolor) << core::endl{bgcolor}
	    << core::endl{ drawKeyInfo ? bgcolor : core::Color::White } << core::endl();

	//core::Color bgcolor = core::Color::Bright_White;
	//std::cout << core::endl{ bgcolor } << core::ColoredStr(title, core::Color::Black, bgcolor) << core::endl{ bgcolor } << core::endl{ bgcolor } << core::endl{ core::Color::White } << core::endl{ };

	for (int i = 0; i < options.size(); ++i) {
		std::cout << std::string(musicList.getPosX(), ' ') << "> ";
		if (i == hover) {
			std::cout << core::ColoredStr(options[i], core::Color::Light_Aqua);
		}
		else if (i == selected) {
			std::cout << core::ColoredStr(options[i], core::Color::Aqua);
		}
		else {
			std::cout << core::ColoredStr(options[i], core::Color::White);
		}
		std::cout << core::ColoredStr(drawKeyInfo && i == 0 ? " [O]" : "", core::Color::Gray) << core::endl();
	}

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

	smallMusicPlayer.draw();

	// Footer:
	{
		int lineCount = core::console::getCharCount().y;
		for (int i = core::console::getCursorPos().y; i < lineCount - 1; ++i) {
			std::cout << core::endl();
		}
		std::cout << core::ColoredStr(drawKeyInfo ? " Hide [K]ey Info" : " Show [K]ey Info", core::Color::Gray, core::Color::White);
		std::cout << core::ColoredStr(std::string(core::console::getCharCount().x - core::console::getCursorPos().x, ' '), core::Color::Gray, core::Color::White);
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