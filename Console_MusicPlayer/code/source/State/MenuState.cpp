#include "State/MenuState.hpp"
#include "Tools/Tool.hpp"
#include "Tools/InputDevice.hpp"
#include "Console.hpp"
#include "Message/Messages.hpp"
#include "App.hpp"
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace fs = std::filesystem;

MenuState::MenuState(App* app) :
	app(app),
	playlistList(),
	options( { "All music", "Playlists", "Directories" }),
	selected(Option::AllMusic),
	hover(Option::None)
{
	
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
	playlistList.init(app, true, 10, 60, selectedPlaylist);
	musicList.init(app);
	directoryList.init(app);

	initMusicList();
	initDirectories();
}

void MenuState::terminate()
{
	// Well, I need the selected playlist in PlayState..
	//musicList.terminate();
	//playlistList.terminate();
	//directoryList.terminate();
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
}

void MenuState::handleEvent()
{
	if (core::inputDevice::isKeyPressed(VK_RETURN)) {
		if (!isInsideOption()) {
			selected = hover;
			// Jump into the option:
			hover = None;
			if (selected == Option::AllMusic) musicList.gainFocus();
			else if (selected == Option::Playlists) playlistList.gainFocus();
			else if (selected == Option::Directories) directoryList.gainFocus();
		}
		else if (selected == Option::AllMusic) {
			// TODO
		}
		else if (selected == Option::Playlists) {
			// Play playlist:
			app->messageBus.send(Message::MenuState_EnteredPlaylist);
			// Update config:
			std::map<std::wstring, std::wstring> config = core::getConfig("data/config.dat");
			config[L"defaultPlaylist"] = std::to_wstring(playlistList.getSelectedIndex());
			core::setConfig("data/config.dat", config);
		}
		else if (selected == Option::Directories) {
			// TODO
		}
		else __debugbreak();

		// TODO: app->messageBus.send(Message::MenuState_SelectedPlaylistEditor);
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
}

void MenuState::draw()
{
	std::cout << core::ColoredStr("Select options:", core::Color::Light_Yellow) << core::endl();
	std::cout << core::ColoredStr("(Select and press enter)", core::Color::Gray) << core::endl() << core::endl();

	for (int i = 0; i < options.size(); ++i) {
		std::cout << "> ";
		if (i == hover) {
			std::cout << core::ColoredStr(options[i], core::Color::Light_Aqua) << core::endl();
		}
		else if (i == selected) {
			std::cout << core::ColoredStr(options[i], core::Color::Gray) << core::endl();
		}
		else {
			std::cout << core::ColoredStr(options[i], core::Color::White) << core::endl();
		}
	}

	std::cout << core::endl() << core::endl();

	if (selected == Option::AllMusic) {
		musicList.draw();
	}
	else if (selected == Option::Playlists) {
		playlistList.draw();
	}
	else if (selected == Option::Directories) {
		directoryList.draw();
	}
}

void MenuState::initMusicList()
{
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

bool MenuState::isInsideOption()
{
	return hover == Option::None;
}

bool MenuState::isInsideOption(Option option)
{
	return isInsideOption() && option == selected;
}

std::filesystem::path MenuState::getPlaylistPath()
{
	if (selected >= options.size()) {
		selected = Option::First;
		__debugbreak();
	}
	return "data/" + playlistList.getSelected() + ".pl";
}