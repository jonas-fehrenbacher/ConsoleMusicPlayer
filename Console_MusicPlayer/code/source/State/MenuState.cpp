#include "State/MenuState.hpp"
#include "Tools/Tool.hpp"
#include "Tools/InputDevice.hpp"
#include "Message/Messages.hpp"
#include "App.hpp"
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace fs = std::filesystem;

MenuState::MenuState(App* app) :
	app(app),
	playlistPaths(),
	selectedPlaylist(1) // minimum is 1
{
	
}

void MenuState::init()
{
	playlistPaths.clear();
	// selectedPlaylist; // Should be the same as the one that got selected.

	// Set selectedPlaylist:
	// Do not do this in the constructor, because in App::App I need to check if the directory exists.
	std::map<std::string, std::string> config = core::getConfig("data/config.dat");
	selectedPlaylist = std::stoi(config["defaultPlaylist"]);
	if (selectedPlaylist <= 0) {
		std::cerr << "Error: config.dat::defaultPlaylist may not be less than 1!\n";
		__debugbreak();
	}
}

void MenuState::terminate()
{

}

void MenuState::update()
{
	// Search for playlists:
	// I'm doing this regularly, so that user can add playlists while program is running.
	playlistPaths.clear();
	playlistPaths.push_back("all"); // virtual playlist (plays all music)
	for (auto& it : fs::directory_iterator("data")) {
		if (it.is_regular_file() && it.path().extension() == ".pl") {
			playlistPaths.push_back(it.path());
		}
	}
}

void MenuState::handleEvent()
{
	if (core::inputDevice::isKeyPressed(VK_RETURN)) {
		app->messageBus.send(Message::MenuState_EnteredPlaylist);

		// Update config:
		std::map<std::string, std::string> config = core::getConfig("data/config.dat");
		config["defaultPlaylist"] = std::to_string(selectedPlaylist);
		core::setConfig("data/config.dat", config);
	}

	for (int i = '0'; i <= '9'; ++i) {
		if (i > '0' && core::inputDevice::isKeyPressed(i)) {
			// Playlists are listed from number 1.
			selectedPlaylist = i - '0';
		}
	}

	if (core::inputDevice::isKeyPressed(VK_UP)) {
		// Playlists are listed from number 1.
		--selectedPlaylist;
		if (selectedPlaylist < 1) {
			selectedPlaylist = playlistPaths.size();
		}
	}

	if (core::inputDevice::isKeyPressed(VK_DOWN)) {
		// Playlists are listed from number 1.
		++selectedPlaylist;
		if (selectedPlaylist > playlistPaths.size()) {
			selectedPlaylist = 1;
		}
	}
}

void MenuState::draw()
{
	std::cout << core::ColoredStr("Your playlists:", core::Color::Light_Yellow) << "\n";
	std::cout << core::ColoredStr("(Select and press enter)", core::Color::Gray) << "\n\n";
	for (int i = 0; i < playlistPaths.size(); ++i) {
		if (i == selectedPlaylist - 1) {
			std::cout << (i+1) << ". " << core::ColoredStr(playlistPaths[i].stem().string(), core::Color::Light_Aqua) << "\n";
		}
		else {
			std::cout << (i+1) << ". " << core::ColoredStr(playlistPaths[i].stem().string(), core::Color::White) << "\n";
		}
		
	}
}

std::filesystem::path MenuState::getPlaylistPath()
{
	if (selectedPlaylist <= 0 || selectedPlaylist > playlistPaths.size()) {
		selectedPlaylist = 1;
		__debugbreak();
	}
	return playlistPaths[selectedPlaylist - 1];
}