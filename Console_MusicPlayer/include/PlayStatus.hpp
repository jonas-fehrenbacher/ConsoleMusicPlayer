#pragma once

#include "core/Console.hpp"
#include "core/Time.hpp"
class App;

class PlayStatus
{
public:
    struct Style
    {
        core::Color background;
        core::Color statusOn;
        core::Color statusOff;
        core::Color durationProgressBar;
        core::Color progressBar_durationText;
        core::Color durationText;
        core::Color progressBar_label;
        core::Color label;
        core::Color skipForwardReport;
        core::Color skipBackwardReport;
        core::Color volumePlusReport;
        core::Color volumeMinusReport;
    };

	void init(App* app);
	void draw();
private:
	App* app;

    /** Position is console cursor position. */
    void drawDurationBar(int size, std::string label, core::Time elapsedTime, core::Time duration, bool isDrawKeyInfo);
};