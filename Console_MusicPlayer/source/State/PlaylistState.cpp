#include "State/PlaylistState.hpp"
#include "App.hpp"
#include "Messages.hpp"
#include "core/SmallTools.hpp"
#include "core/InputDevice.hpp"
#include "core/MusicPlayer.hpp"
#include <map>
#include <Windows.h>
#include <iostream>

PlaylistState::PlaylistState(App* app) :
	app(app),
	playlistList(),
	state(),
	messageReceiverID()
{

}

void PlaylistState::init()
{
	// Add message listener:
	messageReceiverID = app->messageBus.add(std::bind(&PlaylistState::onMessage, this, std::placeholders::_1));

	///////////////////////////////////////////////////////////////////////////////
	// Init playlist list
	///////////////////////////////////////////////////////////////////////////////
	core::DrawableList::Options scrollListFlags = (core::DrawableList::Options)(
		(int)core::DrawableList::SelectionMode |
		//(int)core::DrawableList::ArrowInput | // I need this keys for changing the track
		(int)core::DrawableList::DrawFullX // When this is set then use DrawableList::getPosX()
	);
	core::DrawableList::InitInfo sliInfo;
	sliInfo.options             = scrollListFlags;
	sliInfo.style               = app->style.drawableList;
	sliInfo.name                = "playlists";
	sliInfo.columnLayout        = {};
	sliInfo.spaceBetweenColumns = 3;
	sliInfo.sizeInside          = { 60, 20 };
	sliInfo.hover               = 0; // selectedPlaylist;
	playlistList.init(sliInfo);
	// Fill list:
	for (auto& it : fs::directory_iterator("data")) {
		if (it.is_regular_file() && it.path().extension() == ".pl") {
			// Set playlist list:
			playlistList.push_back({ it.path().stem().string() });

			// Set music player playlists:
			app->musicPlayer.addPlaylist(it.path().u8string());
		}
	}
	// Calculate everything new (important):
	playlistList.onConsoleResize();

	// start:
	app->musicPlayer.resumeDrawableListEvents();
	playlistList.gainFocus();
	scrollToTop();
	app->musicPlayer.setDrawnPlaylist("");
	state = State::PlaylistList;
}

void PlaylistState::terminate()
{
	playlistList.terminate();
	app->messageBus.remove(messageReceiverID);
}

void PlaylistState::update()
{
	// playlist list:
	if (state == State::PlaylistList) 
	{
		std::string selectedPlaylistName = playlistList.getSelectedIndex() == core::DrawableList::NOINDEX ? "" : playlistList.getSelected()[0];
		// Update data:
		playlistList.clear();
		int i = 0;
		for (auto& it : fs::directory_iterator("data")) {
			if (it.is_regular_file() && it.path().extension() == ".pl") {
				playlistList.push_back({ it.path().stem().string() });
				// Select old playlist:
				// Order can change, so this is necessary.
				if (selectedPlaylistName == it.path().stem().string()) {
					playlistList.select(i);
				}
				++i; // increase i here, because in data/ can be other files.
			}
		}
		// Update border color:
		if (app->musicPlayer.isStopped())      playlistList.style.border = core::Color::Light_Red;
		else if (app->musicPlayer.isPaused())  playlistList.style.border = core::Color::Light_Aqua;
		else if (app->musicPlayer.isPlaying()) playlistList.style.border = core::Color::Light_Green;

		playlistList.update();
	}

	if (isTrappedOnTop()) {
		app->musicPlayer.stopDrawableListEvents();
		playlistList.loseFocus();
		app->navBar.gainFocus();
	}
}

void PlaylistState::handleEvent()
{
	///////////////////////////////////////////////////////////////////////////////
	// Back key
	///////////////////////////////////////////////////////////////////////////////
	if (core::inputDevice::isKeyPressed('B')) {
		state = State::PlaylistList;
		app->musicPlayer.setDrawnPlaylist("");
	}

	if (state == State::PlaylistList)
	{
		playlistList.handleEvent();

		if (playlistList.hasFocus() && core::inputDevice::isKeyPressed(VK_RETURN))
		{
			playlistList.selectHoveredItem();
			std::string selectedPlaylistName = playlistList.getSelectedIndex() == core::DrawableList::NOINDEX ? "" : playlistList.getSelected()[0] + ".pl";
			state = State::Playlist;
			app->musicPlayer.setDrawnPlaylist(selectedPlaylistName);
			
			// Update config:
			std::map<std::wstring, std::wstring> config = core::getConfig("data/config.dat");
			config[L"defaultPlaylist"] = std::to_wstring(playlistList.getSelectedIndex());
			core::setConfig("data/config.dat", config);
		}
	}

	app->navBar.handleEvents();
}

void PlaylistState::draw()
{
	app->title.draw();
	app->navBar.draw();

	core::console::setBgColor(app->style.default.bg);
	std::cout << core::endl();

	if (state == State::PlaylistList) {
		playlistList.draw();
	}
	else {
		std::string backInfo = app->isDrawKeyInfo ? " <[B]ack"s : " <";
		std::cout << core::Text(backInfo, core::Color::White) << core::endl();
		app->musicPlayer.draw();
	}

	app->playStatus.draw();
	app->footer.draw();
}

bool PlaylistState::isTrappedOnTop()
{
	bool isTrappedOnTop = false;
	if (state == State::Playlist) {
		isTrappedOnTop = app->musicPlayer.isTrappedOnTop();
	}
	else if (state == State::PlaylistList) {
		isTrappedOnTop = playlistList.isTrappedOnTop();
	}
	return isTrappedOnTop;
}

void PlaylistState::scrollToTop()
{
	if (state == State::Playlist) {
		app->musicPlayer.scrollDrawableListToTop();
	}
	else if (state == State::PlaylistList) {
		playlistList.scrollToTop();
	}
}

void PlaylistState::onMessage(core::Message message)
{
	if (message.id == core::MessageID::CONSOLE_RESIZE) {
		playlistList.onConsoleResize();
	}

	if (message.id == Message::NAVBAR_SHORTCUT_TRIGGERED) {
		app->musicPlayer.stopDrawableListEvents();
		playlistList.loseFocus();
	}

	if (message.id == Message::NAVBAR_BACK && *(NavBar::Option*)message.userData == NavBar::Option::Playlists) {
		app->musicPlayer.resumeDrawableListEvents();
		playlistList.gainFocus();
	}
}