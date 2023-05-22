#include "Footer.hpp"
#include "App.hpp"
#include "core/Console.hpp"
#include "core/InputDevice.hpp"
#include "core/SmallTools.hpp"
#include <iostream>

void Footer::init(App* app)
{
	this->app = app;
}

void Footer::draw()
{
	Style style = app->style.footer;

	// Set background color:
	core::console::setBgColor(style.background);

	int lineCount = core::console::getCharCount().y;
	for (int i = core::console::getCursorPos().y; i < lineCount - 1; ++i) {
		std::cout << core::endl();
	}
	std::cout << " " << core::Text(" " + app->keymap.get(Keymap::Action::KeyInfo).symbol + " ", style.keyShortcut.fg, style.keyShortcut.bg)
		<< core::Text(app->isDrawKeyInfo ? " hide keys" : " show keys", style.keyShortcutText);
	std::cout << std::string(core::console::getCharCount().x - core::console::getCursorPos().x, ' ');
}