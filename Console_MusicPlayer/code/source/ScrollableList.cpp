#include "ScrollableList.hpp"
#include "App.hpp"
#include "Console.hpp"
#include "Tools/Tool.hpp"
#include "Tools/InputDevice.hpp"
#include <sstream>

void core::ScrollableList::initList()
{
	for (auto& musicDir : app->musicDirs) {
		for (auto& it : fs::recursive_directory_iterator(musicDir)) {
			if (!isSupportedAudioFile(it.path())) {
				continue;
			}

			// Open music to get its metadata:
			Mix_Music* music = Mix_LoadMUS(it.path().u8string().c_str());
			if (!music) {
				log("Failed to load music! SDL_mixer Error: " + std::string(Mix_GetError()));
				//__debugbreak();
				return;
			}

			std::string filenameStem = it.path().stem().u8string();
			std::string sdlTitle = Mix_GetMusicTitle(music);
			std::string title = strcmp(Mix_GetMusicTitle(music), "") == 0 ? filenameStem : Mix_GetMusicTitle(music); // SDL2 does not return filename as mentioned, so I do it manually.
			Time duration = Time(Seconds((int)Mix_MusicDuration(music)));
			list.push_back(title);
			Mix_FreeMusic(music);
		}
	}
}

void core::ScrollableList::init(App* app, int maxDrawnItems /*= 10*/, int maxDrawnItemNameLength /*= 60*/, size_t selected /*= 0*/)
{
	this->app = app;
	this->maxDrawnItems = maxDrawnItems;
	this->maxDrawnItemNameLength = maxDrawnItemNameLength;
	selected = 0;
	startDrawIndex = 0;
	initList();
}

void core::ScrollableList::terminate()
{
	list.clear();
	selected = 0;
}

void core::ScrollableList::update()
{
	// TODO: initList every few seconds... maybe in another thread.
}

void core::ScrollableList::handleEvent()
{
	std::vector<inputDevice::MouseWheelScroll> mouseWheelScrollEvents = inputDevice::getMouseWheelScrollEvents();
	for (auto& mouseWheelScrollEvent : mouseWheelScrollEvents) {
		if (mouseWheelScrollEvent == inputDevice::MouseWheelScroll::Up) {
			if (startDrawIndex > 0)
				--startDrawIndex;
		}
		else {
			if (startDrawIndex < list.size() - maxDrawnItems)
			++startDrawIndex;
		}
	}
}

intern void drawBorder(int size)
{
	std::string border = "";
	for (int i = 0; i < size; ++i) {
		border += "_";
	}
	std::cout << "\t" << core::ColoredStr(border, core::Color::Gray) << core::endl();
}

void core::ScrollableList::draw()
{

	drawBorder(maxDrawnItemNameLength + 3 + 5); // +3: "...", +3: "300. "
	//std::cout << core::endl();

	if (list.empty()) {
		std::cout << "\tNothing found!" << core::endl();
	}
	else {
		core::ColoredStr line("", Color::White);
		std::stringstream ss;
		for (size_t i = startDrawIndex; i < list.size() && i < startDrawIndex + maxDrawnItems; ++i) {
			// Set line text:
			ss.str("");
			ss << "\t" << std::left << std::setw(5) << std::to_string((i + 1)) + "." << list[i].substr(0, maxDrawnItemNameLength) 
				<< (list[i].length() > maxDrawnItemNameLength ? "..." : ""); // core::endl() does not work for last item here, so I use it bellow.
			line = ss.str();
			// Set line color:
			const int LAST_DRAWN_ITEM_INDEX = startDrawIndex + (maxDrawnItems - 1);
			bool isFirstDrawn = i == startDrawIndex;
			bool isFirstItem = i == 0;
			bool isLastDrawn = i == LAST_DRAWN_ITEM_INDEX;
			bool isLastItem = i == list.size() - 1;
			if (((isFirstDrawn && !isFirstItem) || (isLastDrawn && !isLastItem)) && list.size() > maxDrawnItems) {
				// ...drawn item is at the top or bottom, but you can scroll higher or lower.
				line.color = Color::Gray;
			}
			else {
				line.color = Color::White;
			}
			// Output:
			std::cout << line << core::endl();
		}
	}
	std::cout.flush();
	drawBorder(maxDrawnItemNameLength + 3 + 5);
}

void core::ScrollableList::clear()
{
	list.clear();
}

void core::ScrollableList::push_back(std::string item)
{
	list.push_back(item);
}

size_t core::ScrollableList::size()
{
	return list.size();
}

size_t core::ScrollableList::getSelectedIndex()
{
	return selected;
}

std::string core::ScrollableList::getSelected()
{
	return list.at(selected);
}