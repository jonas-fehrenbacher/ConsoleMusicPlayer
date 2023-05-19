#pragma once

#include "core/Console.hpp"
class App;

class PlayStatus
{
public:
    struct Style
    {
        core::Color statusOn;
        core::Color statusOff;
        core::Color durationProgressBar;
        core::Color durationProgressBarText;
        core::Color durationText;
    };

	void init(App* app);
	void draw();
private:
	App* app;
};