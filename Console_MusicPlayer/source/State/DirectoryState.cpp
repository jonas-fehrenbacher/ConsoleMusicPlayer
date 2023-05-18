#include "State/DirectoryState.hpp"
#include "App.hpp"
#include "core/SmallTools.hpp"
#include "core/InputDevice.hpp"
#include <map>
#include <Windows.h>

void DirectoryState::init(App* app)
{
	this->app = app;

	///////////////////////////////////////////////////////////////////////////////
	// Init list
	///////////////////////////////////////////////////////////////////////////////
	core::ScrollableList::Options scrollListFlags = (core::ScrollableList::Options)(
		(int)core::ScrollableList::SelectionMode |
		//(int)core::ScrollableList::ArrowInput | // I need this keys for changing the track
		(int)core::ScrollableList::DrawFullX // When this is set then use ScrollableList::getPosX()
	);
	core::ScrollableList::InitInfo sliInfo;
	sliInfo.options             = scrollListFlags;
	sliInfo.style               = app->style.scrollableList;
	sliInfo.name                = "directories";
	sliInfo.columnLayout        = {};
	sliInfo.spaceBetweenColumns = 3;
	sliInfo.sizeInside          = { 60, 20 };
	sliInfo.hover               = 0;
	list.init(sliInfo);

	for (auto& musicDir : app->musicDirs) {
		list.push_back({ musicDir.u8string() });
	}

	// Calculate everything new (important):
	list.onConsoleResize();
}

void DirectoryState::terminate()
{
	list.terminate();
}

void DirectoryState::update()
{
	list.update();

	// Update border color:
	if (!Mix_PlayingMusic())                           list.style.border = core::Color::Light_Red; // stopped
	else if (Mix_PausedMusic())                        list.style.border = core::Color::Light_Aqua; // paused
	else if (Mix_PlayingMusic() && !Mix_PausedMusic()) list.style.border = core::Color::Light_Green; // playing
}

void DirectoryState::handleEvent()
{
	list.handleEvent();

	if (core::inputDevice::isKeyPressed(VK_RETURN))
	{
		list.selectHoveredItem();
	}
}

void DirectoryState::draw()
{
	list.draw();
}

void DirectoryState::loseFocus()
{
	list.loseFocus();
}

void DirectoryState::gainFocus()
{
	list.gainFocus();
}

bool DirectoryState::isTrappedOnTop()
{
	return list.isTrappedOnTop();
}

void DirectoryState::scrollToTop()
{
	list.scrollToTop();
}

void DirectoryState::onConsoleResize()
{
	list.onConsoleResize();
}