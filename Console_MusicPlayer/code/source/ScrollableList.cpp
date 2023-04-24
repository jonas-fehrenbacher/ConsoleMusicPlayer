#include "ScrollableList.hpp"
#include "App.hpp"
#include "Console.hpp"
#include "Tools/Tool.hpp"
#include "Tools/InputDevice.hpp"
#include <sstream>
#include <Windows.h>

void core::ScrollableList::init(App* app, bool selectionMode /*= true*/, int maxDrawnItems /*= 10*/, int maxDrawnItemNameLength /*= 60*/, size_t selected /*= 0*/)
{
	this->app = app;
	this->maxDrawnItems = maxDrawnItems;
	this->maxDrawnItemNameLength = maxDrawnItemNameLength;
	this->selectionMode = selectionMode;
	this->selected = selected;
	drawnItemsSelectionPos = 0;
	isTrappedOnTop_ = false;
	isTrappedOnBottom_ = false;
	startDrawIndex = 0;
	hasFocus = true;
}

void core::ScrollableList::terminate()
{
	list.clear();
	startDrawIndex = 0;
	selected = 0;
	selectionMode = true;
	drawnItemsSelectionPos = 0;
	isTrappedOnTop_ = false;
	isTrappedOnBottom_ = false;
	hasFocus = true;
}

void core::ScrollableList::update()
{
	if (!hasFocus) {
		return;
	}

	// TODO: initList every few seconds... maybe in another thread.
}

void core::ScrollableList::move(bool up)
{
	if (selectionMode)
	{
		if (up) {
			if (selected > 0) {
				--selected;
				isTrappedOnBottom_ = false;
			}
			else {
				isTrappedOnTop_ = true;
			}

			if (startDrawIndex > 0 && selected < list.size() - maxDrawnItems && drawnItemsSelectionPos == 0)
				--startDrawIndex;
			if (drawnItemsSelectionPos > 0)
				--drawnItemsSelectionPos;
		}
		else {
			if (selected < list.size() - 1) {
				++selected;
				isTrappedOnTop_ = false;
			}
			else {
				isTrappedOnBottom_ = true;
			}

			if (startDrawIndex < list.size() - maxDrawnItems && selected >= maxDrawnItems && drawnItemsSelectionPos == maxDrawnItems - 1)
				++startDrawIndex;
			if (drawnItemsSelectionPos < maxDrawnItems - 1)
				++drawnItemsSelectionPos;
		}
	}
	else
	{
		// ...selection is ignored
		if (up) {
			if (startDrawIndex > 0)
				--startDrawIndex;
		}
		else {
			if (startDrawIndex < list.size() - maxDrawnItems)
				++startDrawIndex;
		}
	}
}

void core::ScrollableList::handleEvent()
{
	if (!hasFocus) {
		return;
	}

	// Move list items:
	std::vector<inputDevice::MouseWheelScroll> mouseWheelScrollEvents = inputDevice::getMouseWheelScrollEvents();
	for (auto& mouseWheelScrollEvent : mouseWheelScrollEvents) {
		move(mouseWheelScrollEvent == inputDevice::MouseWheelScroll::Up);
	}
	if (core::inputDevice::isKeyPressed(VK_UP)) {
		move(true);
	}
	else if (core::inputDevice::isKeyPressed(VK_DOWN)) {
		move(false);
	}

	if (selectionMode && core::inputDevice::isKeyPressed(VK_RETURN))
	{
		// Should I do something here with the selected item?
	}
}

intern void drawBorder(int size)
{
	std::string border = "";
	for (int i = 0; i < size; ++i) {
		border += "_";
	}
	std::cout << core::tab() << core::ColoredStr(border, core::Color::Gray) << core::endl();
}

void core::ScrollableList::draw()
{

	drawBorder(maxDrawnItemNameLength + 3 + 5); // +3: "...", +3: "300. "
	//std::cout << core::endl();

	if (list.empty()) {
		std::cout << core::tab() << "Nothing found!" << core::endl();
	}
	else {
		core::ColoredStr line("", Color::White);
		std::stringstream ss;
		for (size_t i = startDrawIndex; i < list.size() && i < startDrawIndex + maxDrawnItems; ++i) {
			// Set line text:
			ss.str("");
			ss << core::tab() << std::left << std::setw(5) << std::to_string((i + 1)) + "." << list[i].substr(0, maxDrawnItemNameLength)
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
			if (hasFocus && selectionMode && i == selected) {
				line.color = Color::Light_Aqua;
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

void core::ScrollableList::loseFocus()
{
	hasFocus = false;
	isTrappedOnBottom_ = false;
	isTrappedOnTop_ = false;
}

void core::ScrollableList::gainFocus()
{
	hasFocus = true;
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

bool core::ScrollableList::isTrappedOnTop()
{
	return isTrappedOnTop_;
}

bool core::ScrollableList::isTrappedOnBottom()
{
	return isTrappedOnBottom_;
}