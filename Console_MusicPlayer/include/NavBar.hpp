#pragma once

#include "core/Console.hpp"
#include <array>
#include <string>
class App;

/** Navigation bar */
class NavBar
{
public:
	enum Option
    {
        AllMusic    = 0,
        Playlists   = 1,
        Directories = 2,

        None  = 3, //< If 'hover' is equal 'None', then we are currently in the music list, playlist list or directory list.
        First = 0,
        Last  = 2
    };

    struct Style
    {
        core::Color     background;
        core::Color     keyInfo;
        core::FullColor item;
        core::FullColor item_hover;
        core::FullColor item_selected;
    };

	void init(App* app);
	void handleEvents();
	void draw();
	/** select last element and start handling events. */
	void gainFocus();
private:
	App*                       app;
	std::array<std::string, 3> options;
	Option                     selected; //< this item is selected
	Option                     hover; //< over this item do we hover

	bool isInsideNavBar();
};