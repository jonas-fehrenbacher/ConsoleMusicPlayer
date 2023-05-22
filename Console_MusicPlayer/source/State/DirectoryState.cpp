#include "State/DirectoryState.hpp"
#include "App.hpp"
#include "Messages.hpp"
#include "core/SmallTools.hpp"
#include "core/InputDevice.hpp"
#include <map>
#include <Windows.h>
#include <iostream>

DirectoryState::DirectoryState(App* app) :
	app(app),
	list(),
	messageReceiverID()
{

}

void DirectoryState::init()
{
	// Add message listener:
	messageReceiverID = app->messageBus.add(std::bind(&DirectoryState::onMessage, this, std::placeholders::_1));

	///////////////////////////////////////////////////////////////////////////////
	// Init list
	///////////////////////////////////////////////////////////////////////////////
	core::DrawableList::Options scrollListFlags = (core::DrawableList::Options)(
		(int)core::DrawableList::SelectionMode |
		//(int)core::DrawableList::ArrowInput | // I need this keys for changing the track
		(int)core::DrawableList::DrawFullX // When this is set then use DrawableList::getPosX()
	);
	core::DrawableList::InitInfo sliInfo;
	sliInfo.options             = scrollListFlags;
	sliInfo.style               = app->style.drawableList;
	sliInfo.name                = "directories";
	sliInfo.columnLayout        = {};
	sliInfo.spaceBetweenColumns = 3;
	sliInfo.sizeInside          = { 60, 20 };
	sliInfo.hover               = 0;
	list.init(sliInfo);
	// Fill list:
	for (auto& musicDir : app->musicDirs) {
		list.push_back({ musicDir.u8string() });
	}
	// Calculate everything new (important):
	list.onConsoleResize();

	// start:
	update();
	list.gainFocus();
	list.scrollToTop();
}

void DirectoryState::terminate()
{
	list.terminate();
	app->messageBus.remove(messageReceiverID);
}

void DirectoryState::update()
{
	list.update();

	// Update border color:
	if (!Mix_PlayingMusic())                           list.style.border = core::Color::Light_Red; // stopped
	else if (Mix_PausedMusic())                        list.style.border = core::Color::Light_Aqua; // paused
	else if (Mix_PlayingMusic() && !Mix_PausedMusic()) list.style.border = core::Color::Light_Green; // playing

	if (list.isTrappedOnTop()) {
		list.loseFocus();
		app->navBar.gainFocus();
	}
}

void DirectoryState::handleEvent()
{
	list.handleEvent();

	if (list.hasFocus() && core::inputDevice::isKeyPressed(app->keymap.get(Keymap::Action::Select).key))
	{
		list.selectHoveredItem();
	}

	app->navBar.handleEvents();
}

void DirectoryState::draw()
{
	app->title.draw();
	app->navBar.draw();

	core::console::setBgColor(app->style.default.bg);
	std::cout << core::endl();
	list.draw();

	app->playStatus.draw();
	app->footer.draw();
}

void DirectoryState::onMessage(core::Message message)
{
	if (message.id == core::MessageID::CONSOLE_RESIZE) {
		list.onConsoleResize();
	}

	if (message.id == Message::NAVBAR_SHORTCUT_TRIGGERED) {
		list.loseFocus();
	}

	if (message.id == Message::NAVBAR_BACK && *(NavBar::Option*)message.userData == NavBar::Option::Directories) {
		list.gainFocus();
	}
}