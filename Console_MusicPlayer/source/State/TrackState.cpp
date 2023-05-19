#include "State/TrackState.hpp"
#include "Messages.hpp"
#include "App.hpp"
#include "core/SmallTools.hpp"
#include "core/InputDevice.hpp"
#include "core/Console.hpp"
#include "core/MusicPlayer.hpp"
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

TrackState::TrackState(App* app) :
	app(app),
	messageReceiverID()
{
	
}

void TrackState::init()
{
	// Add message listener:
	// This may not be called in the constructor, because App::messagebus is at this time not initialized.
	// Options: 'bool isFirstInit' or TrackState::terminate()::messageBus.remove()
	messageReceiverID = app->messageBus.add(std::bind(&TrackState::onMessage, this, std::placeholders::_1));
	
	app->musicPlayer.resumeDrawableListEvents();
	app->musicPlayer.scrollDrawableListToTop();
	app->musicPlayer.setDrawnPlaylist(core::MusicPlayer::ALL_PLAYLIST_NAME);
}

void TrackState::terminate()
{
	app->messageBus.remove(messageReceiverID);
}

void TrackState::update()
{
	if (app->musicPlayer.isTrappedOnTop()) {
		app->musicPlayer.stopDrawableListEvents();
		app->navBar.gainFocus();
	}
}

void TrackState::handleEvent()
{
	app->navBar.handleEvents();
}

void TrackState::draw()
{
	app->title.draw();
	app->navBar.draw();

	///////////////////////////////////////////////////////////////////////////////
	// Draw scrollable list
	///////////////////////////////////////////////////////////////////////////////
	core::console::setBgColor(app->style.default.bg);
	std::cout << core::endl();
	app->musicPlayer.draw();

	app->playStatus.draw();
	app->footer.draw();
}

void TrackState::onMessage(core::Message message)
{
	if (message.id == core::MessageID::CONSOLE_RESIZE) {
		
	}

	if (message.id == Message::NAVBAR_SHORTCUT_TRIGGERED) {
		app->musicPlayer.stopDrawableListEvents();
	}

	if (message.id == Message::NAVBAR_BACK && *(NavBar::Option*)message.userData == NavBar::Option::AllMusic) {
		app->musicPlayer.resumeDrawableListEvents();
	}
}