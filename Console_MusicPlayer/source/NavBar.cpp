#include "NavBar.hpp"
#include "App.hpp"
#include "Messages.hpp"
#include "core/Console.hpp"
#include "core/InputDevice.hpp"
#include "core/SmallTools.hpp"
#include <iostream>
#include <Windows.h>

void NavBar::init(App* app)
{
	this->app = app;
	options   = { "Tracks", "Playlists", "Directories" };
	selected  = Option::AllMusic;
	hover     = Option::None;
}

void NavBar::handleEvents()
{
	///////////////////////////////////////////////////////////////////////////////
	// Jump to navigation bar
	///////////////////////////////////////////////////////////////////////////////
	if (core::inputDevice::isKeyPressed('O')) {
		hover = Option::First;
		app->messageBus.send(Message::NAVBAR_SHORTCUT_TRIGGERED); // Every state needs to listen to this and set themselves off focus.
		// Alternative: set a flag 'bool isShortcutTriggered()' - reset this if 'O' is not pressed.
	}

	///////////////////////////////////////////////////////////////////////////////
	// Navigation bar slection
	///////////////////////////////////////////////////////////////////////////////
	if (isInsideNavBar() && core::inputDevice::isKeyPressed(VK_RETURN))
	{
		selected = hover;
		hover    = Option::None; // Jump into the option

		Option* userData = new Option(selected);
		app->messageBus.send(Message::NAVBAR_OPTION_SELECTED, userData);
	}

	///////////////////////////////////////////////////////////////////////////////
	// Navigation bar selection movement
	///////////////////////////////////////////////////////////////////////////////
	if (isInsideNavBar())
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
					hover = Option::None;

					Option* userData = new Option(selected);
					app->messageBus.send(Message::NAVBAR_BACK, userData); // resume old state - no hard reset like in 'NAVBAR_OPTION_SELECTED'.
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
		// up / down keys are reserved for music player:
		//if (core::inputDevice::isKeyPressed(VK_UP)) {
		//	moveUp();
		//}
		//if (core::inputDevice::isKeyPressed(VK_DOWN)) {
		//	moveDown();
		//}
	}
}

bool NavBar::isInsideNavBar()
{
	return hover != Option::None;
}

void NavBar::draw()
{
	Style style = app->style.navBar;

	// Set background color:
	core::console::setBgColor(style.background);

	// options:
	std::string optionsKeyInfo = app->isDrawKeyInfo ? "[O] " : "";
	int optionSpaceInbetween = 3;
	int optionsSize = 0;
	for (int i = 0; i < options.size(); ++i) optionsSize += options[i].size();
	optionsSize += (options.size() - 1) * optionSpaceInbetween; // space inbetween
	optionsSize += 2; // at the beginning and end is " " for highlighting
	optionsSize += optionsKeyInfo.length();
	int drawPos = core::console::getCharCount().x / 2.f - optionsSize / 2.f;
	std::cout << std::string(drawPos, ' ');

	for (int i = 0; i < options.size(); ++i) {
		core::Text next = core::Text(" " + options[i] + " ");
		if (i == hover) {
			next.fgcolor = style.item_hover.fg; 
			next.bgcolor = style.item_hover.bg;
		}
		else if (i == selected) {
			next.fgcolor = style.item_selected.fg;
			next.bgcolor = style.item_selected.bg;
		}
		else {
			next.fgcolor = style.item.fg;
			next.bgcolor = style.item.bg;
		}
		std::cout 
			<< core::Text(i == 0 ? optionsKeyInfo : "", style.keyInfo)
			<< next
			<< std::string(i == options.size() - 1? 0 : optionSpaceInbetween - 2, ' ');
	}
	std::cout << core::endl();
}

void NavBar::gainFocus()
{
	hover = Option::Last;
}