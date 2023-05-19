#pragma once

#include "core/Console.hpp"

class App;

class Title
{
public:
	struct Style
	{
		core::Color background;
		core::Color title;
		core::Color trackNumber;
		core::Color keyInfo;
		core::Color exitSymbol;
		core::Color lockStatus;
	};

	void init(App* app);
	void draw();
private:
	App* app;
};