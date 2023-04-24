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

enum {
	//PLAYLIST_EDITOR = 0,
	FRONT_PLAYLIST // "all"
};

MenuState::MenuState(App* app) :
	app(app),
	options(),
	selected(1) // minimum is 1 with PLAYLIST_EDITOR
{
	
}

void MenuState::init()
{
	options.clear();
	// selectedPlaylist; // Should be the same as the one that got selected.

	// Set selected:
	// Do not do this in the constructor, because in App::App I need to check if the directory exists.
	std::map<std::wstring, std::wstring> config = core::getConfig("data/config.dat");
	if (config.count(L"defaultPlaylist") == 0) {
		core::log("Error: defaultPlaylist not found in config.dat");
		__debugbreak();
	}
	selected = std::stoi(config[L"defaultPlaylist"]);
	//if (selected <= 0) {
	//	std::cerr << "Error: config.dat::defaultPlaylist may not be less than 1!\n";
	//	__debugbreak();
	//}

	musicList.init(app);
}

void MenuState::terminate()
{
	musicList.terminate();
}

void MenuState::update()
{
	// Search for playlists:
	// I'm doing this regularly, so that user can add playlists while program is running.
	options.clear();
	//options.push_back("Playlist editor");
	options.push_back("all"); // virtual playlist (plays all music)
	for (auto& it : fs::directory_iterator("data")) {
		if (it.is_regular_file() && it.path().extension() == ".pl") {
			options.push_back(it.path().stem().string());
		}
	}


	musicList.update();
}

void MenuState::handleEvent()
{
	if (core::inputDevice::isKeyPressed(VK_RETURN)) {
		// Play playlist:
		if (selected >= FRONT_PLAYLIST) {
			app->messageBus.send(Message::MenuState_EnteredPlaylist);
			// Update config:
			std::map<std::wstring, std::wstring> config = core::getConfig("data/config.dat");
			config[L"defaultPlaylist"] = std::to_wstring(selected);
			core::setConfig("data/config.dat", config);
		}
		else {
			app->messageBus.send(Message::MenuState_SelectedPlaylistEditor);
		}
	}

	// Number input:
	for (int i = '0'; i <= '9'; ++i) {
		if (i > '0' && core::inputDevice::isKeyPressed(i)) {
			// Playlists are listed from number 1.
			selected = FRONT_PLAYLIST + (i - '0');
		}
	}

	if (core::inputDevice::isKeyPressed(VK_UP)) {
		// Playlists are listed from number 1.
		--selected;
		if (selected < 0) {
			selected = options.size() - 1;
		}
	}

	if (core::inputDevice::isKeyPressed(VK_DOWN)) {
		// Playlists are listed from number 1.
		++selected;
		if (selected >= options.size()) {
			selected = 0;
		}
	}

	musicList.handleEvent();
}

void MenuState::draw()
{
	std::cout << core::ColoredStr("Select options:", core::Color::Light_Yellow) << core::endl();
	std::cout << core::ColoredStr("(Select and press enter)", core::Color::Gray) << core::endl() << core::endl();

	for (int i = 0; i < options.size(); ++i) {
		if (i >= FRONT_PLAYLIST) {
			std::cout << (i - (FRONT_PLAYLIST - 1)) << " ";
		}
		else {
			std::cout << "> ";
		}
		if (i == selected) {
			std::cout << core::ColoredStr(options[i], core::Color::Light_Aqua) << core::endl();
		}
		else {
			std::cout << core::ColoredStr(options[i], core::Color::White) << core::endl();
		}
	}

	std::cout << core::endl() << core::endl();
	musicList.draw();
}

std::filesystem::path MenuState::getPlaylistPath()
{
	if (selected < FRONT_PLAYLIST || selected >= options.size()) {
		selected = FRONT_PLAYLIST;
		__debugbreak();
	}
	return "data/" + options[selected] + ".pl";
}