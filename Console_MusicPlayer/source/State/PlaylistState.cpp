#include "State/PlaylistState.hpp"
#include "App.hpp"
#include "core/SmallTools.hpp"
#include "core/InputDevice.hpp"
#include "core/MusicPlayer.hpp"
#include <map>
#include <Windows.h>
#include <iostream>

// TODO: sync playlist with TrackState::playlist via messageBus

void PlaylistState::init(App* app, core::MusicPlayer* musicPlayer)
{
	state = State::PlaylistList;
	this->app = app;
	this->musicPlayer = musicPlayer;
	drawKeyInfo = true;

	///////////////////////////////////////////////////////////////////////////////
	// Init playlist list
	///////////////////////////////////////////////////////////////////////////////
	core::ScrollableList::Options scrollListFlags = (core::ScrollableList::Options)(
		(int)core::ScrollableList::SelectionMode |
		//(int)core::ScrollableList::ArrowInput | // I need this keys for changing the track
		(int)core::ScrollableList::DrawFullX // When this is set then use ScrollableList::getPosX()
	);
	core::ScrollableList::InitInfo sliInfo;
	sliInfo.options             = scrollListFlags;
	sliInfo.style               = app->style.scrollableList;
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
			musicPlayer->addPlaylist(it.path().u8string());
		}
	}
	// Calculate everything new (important):
	playlistList.onConsoleResize();
}

void PlaylistState::terminate()
{
	playlistList.terminate();
}

void PlaylistState::update()
{
	// playlist list:
	if (state == State::PlaylistList) 
	{
		std::string selectedPlaylistName = playlistList.getSelectedIndex() == core::ScrollableList::NOINDEX ? "" : playlistList.getSelected()[0];
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
		if (musicPlayer->isStopped())      playlistList.style.border = core::Color::Light_Red;
		else if (musicPlayer->isPaused())  playlistList.style.border = core::Color::Light_Aqua;
		else if (musicPlayer->isPlaying()) playlistList.style.border = core::Color::Light_Green;

		playlistList.update();
	}
}

void PlaylistState::handleEvent()
{
	///////////////////////////////////////////////////////////////////////////////
	// Back key
	///////////////////////////////////////////////////////////////////////////////
	if (core::inputDevice::isKeyPressed('B')) {
		state = State::PlaylistList;
		musicPlayer->setDrawnPlaylist("");
	}

	if (state == State::PlaylistList)
	{
		playlistList.handleEvent();

		if (core::inputDevice::isKeyPressed(VK_RETURN))
		{
			playlistList.selectHoveredItem();
			std::string selectedPlaylistName = playlistList.getSelectedIndex() == core::ScrollableList::NOINDEX ? "" : playlistList.getSelected()[0] + ".pl";
			state = State::Playlist;
			musicPlayer->setDrawnPlaylist(selectedPlaylistName);
			
			// Update config:
			std::map<std::wstring, std::wstring> config = core::getConfig("data/config.dat");
			config[L"defaultPlaylist"] = std::to_wstring(playlistList.getSelectedIndex());
			core::setConfig("data/config.dat", config);
		}
	}
}

void PlaylistState::draw()
{
	// Draw Back key:
	if (state != State::PlaylistList) {
		std::string backInfo = drawKeyInfo ? " <[B]ack"s : " <";
		std::cout << core::Text(backInfo, core::Color::White) << core::endl();
		musicPlayer->draw();
	}

	if (state == State::PlaylistList) {
		playlistList.draw();
	}
}

void PlaylistState::start()
{
	musicPlayer->resumeDrawableListEvents();
	playlistList.gainFocus();
	scrollToTop();
	musicPlayer->setDrawnPlaylist("");
	state = State::PlaylistList;
}

void PlaylistState::loseFocus()
{
	musicPlayer->stopDrawableListEvents();
	playlistList.loseFocus();
}

void PlaylistState::gainFocus()
{
	musicPlayer->resumeDrawableListEvents();
	playlistList.gainFocus();
}

bool PlaylistState::isTrappedOnTop()
{
	bool isTrappedOnTop = false;
	if (state == State::Playlist) {
		isTrappedOnTop = musicPlayer->isTrappedOnTop();
	}
	else if (state == State::PlaylistList) {
		isTrappedOnTop = playlistList.isTrappedOnTop();
	}
	return isTrappedOnTop;
}

void PlaylistState::scrollToTop()
{
	if (state == State::Playlist) {
		musicPlayer->scrollDrawableListToTop();
	}
	else if (state == State::PlaylistList) {
		playlistList.scrollToTop();
	}
}

void PlaylistState::onConsoleResize()
{
	playlistList.onConsoleResize();
}

void PlaylistState::setDrawKeyInfo(bool drawKeyInfo)
{
	this->drawKeyInfo = drawKeyInfo;
}