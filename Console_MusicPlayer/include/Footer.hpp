#pragma once

#include "core/Console.hpp"
class App;

class Footer
{
public:
    struct Style
    {
        core::Color     background;
        core::FullColor keyShortcut;
        core::Color     keyShortcutText;
    };

	void init(App* app);
	void draw();
private:
	App* app;
};