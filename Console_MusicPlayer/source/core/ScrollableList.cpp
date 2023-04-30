#include "core/ScrollableList.hpp"
#include "core/Console.hpp"
#include "core/SmallTools.hpp"
#include "core/InputDevice.hpp"
#include <sstream>
#include <Windows.h>
#include <algorithm>
#include <cassert>

namespace core {
	intern const size_t NOINDEX = -1; // -1 stands for no index (see 'selected')
}

void core::ScrollableList::init(Options options, int maxDrawnItems /*= 10*/, int maxDrawnItemNameLength /*= 60*/, size_t hover /*= 0*/)
{
	this->maxDrawnItems = maxDrawnItems;
	this->maxDrawnItemNameLength = maxDrawnItemNameLength;
	this->hover = hover;
	this->selected = NOINDEX;
	drawnItemsSelectionPos = 0;
	isTrappedOnTop_ = false;
	isTrappedOnBottom_ = false;
	startDrawIndex = 0;
	hasFocus = true;
	this->options = options;

	onConsoleResize();
}

void core::ScrollableList::terminate()
{
	list.clear();
	startDrawIndex = 0;
	hover = 0;
	posX = 0;
	selected = NOINDEX;
	options = (Options)0;
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
	if (hasFlag(Options::ArrowInput, options)) {
		if (core::inputDevice::isKeyPressed(VK_UP)) {
			move(true);
		}
		else if (core::inputDevice::isKeyPressed(VK_DOWN)) {
			move(false);
		}
	}

	// DO not uncomment this, because it blocks the VK_RETURN event from SmallMusicPlayer...
	//if ( hasFlag(Options::SelectionMode, options) && core::inputDevice::isKeyPressed(VK_RETURN))
	//{
	//	// Should I do something here with the selected item?
	//}
}

void core::ScrollableList::move(bool up)
{
	if (hasFlag(Options::SelectionMode, options))
	{
		if (up) {
			if (hover > 0) {
				--hover;
				isTrappedOnBottom_ = false;
			}
			else {
				isTrappedOnTop_ = true;
			}

			if (startDrawIndex > 0 && hover < list.size() - maxDrawnItems && drawnItemsSelectionPos == 0)
				--startDrawIndex;
			if (drawnItemsSelectionPos > 0)
				--drawnItemsSelectionPos;
		}
		else {
			if (hover < list.size() - 1) {
				++hover;
				isTrappedOnTop_ = false;
			}
			else {
				isTrappedOnBottom_ = true;
			}

			if (startDrawIndex < list.size() - maxDrawnItems && hover >= maxDrawnItems && drawnItemsSelectionPos == maxDrawnItems - 1)
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

intern void drawBorder(int size, bool isTop, int posX);
void core::ScrollableList::draw()
{
	drawBorder(getDrawSize(), true, posX);
	//std::cout << core::endl();

	if (maxDrawnItems <= 0) {
		// do nothing (but can cause an error if used bellow.)
	}
	else if (list.empty()) {
		std::cout << std::string(posX, ' ') << "Nothing found!" << core::endl();
	}
	else {
		core::ColoredStr line("", Color::White);
		std::stringstream ss;
		bool colorState = true;
		int drawnItemCount = list.size() < maxDrawnItems ? list.size() : maxDrawnItems;
		for (size_t i = startDrawIndex; i < startDrawIndex + drawnItemCount; ++i, colorState = !colorState) {
			// Set line text:
			ss.str("");
			std::string name = list[i].substr(0, maxDrawnItemNameLength) + (list[i].length() > maxDrawnItemNameLength ? "..." : "");
			ss << std::string(posX, ' ') << std::left << std::setw(5) << std::to_string((i + 1)) + "." << name;
			// core::endl() does not work for last item here, so I use it bellow.
			line = ss.str();
			// Set line color:
			{
				const int LAST_DRAWN_ITEM_INDEX = startDrawIndex + (maxDrawnItems - 1);
				// TODO: can I not delete isFirstItem and isLastDrawn?
				bool isFirstDrawn = i == startDrawIndex;
				bool isFirstItem = i == 0;
				bool isLastDrawn = i == LAST_DRAWN_ITEM_INDEX;
				bool isLastItem = i == list.size() - 1;
				if (((isFirstDrawn && !isFirstItem) || (isLastDrawn && !isLastItem)) && list.size() > maxDrawnItems) {
					// ...drawn item is at the top or bottom, but you can scroll higher or lower.
					line.fgcolor = Color::Gray;
				}
				else {
					line.fgcolor = Color::White; // colorState ? Color::White : Color::Bright_White;
				}
				if (hasFocus && hasFlag(Options::SelectionMode, options) && i == hover) {
					assert(hover != NOINDEX);
					line.fgcolor = Color::Light_Aqua; // Light_Green, Light_Aqua; Light_Yellow, Bright_White
				}
				else if (hasFlag(Options::SelectionMode, options) && i == selected) {
					line.fgcolor = Color::Aqua;
				}
			}
			// Output:
			std::cout << line;

			// Output scrollbar:
			// - Important: Unfortunately, std::setw() does not work for the scrollbar, because the music names have unicode characters and
			//   std::stringstream does not work properly with that (using std::wstringstream is to troublesome).
			// - Calculation:
			//   1. scrollbarOriginPos: Scrollbar position if it would be 1 character in size (is the center on a larger scrollbar).
			//   2. scrollbarSize: How large the scrollbar needs to be.
			//   3. scrollbarTop_itemIndex, scrollbarBottom_itemIndex: Start draw position and end draw position on the y axis.
			int middleDrawnItemIndex = round(startDrawIndex + maxDrawnItems / 2.f); // using 'hover' is not that good if there are few items.
			float scrollbarPosFactor = middleDrawnItemIndex / (float)list.size(); // 0..1
			float scrollbarOriginPosRaw = (drawnItemCount - 1) * scrollbarPosFactor; // 0..drawnItemCount-1; Position if scrollbarSize==1
			int scrollbarOriginPos = (startDrawIndex + scrollbarOriginPosRaw) > list.size() / 2.f ? round(scrollbarOriginPosRaw) : scrollbarOriginPosRaw;
			// (maxDrawnItems-1): This is neccessray, because we want 'maxDrawnItems' positions (0..MDI-1) and not 'maxDrawnItems+1' positions.
			float scrollbarSizeFactor = drawnItemCount / (float)list.size();
			int scrollbarSize = std::clamp((int)round(maxDrawnItems * scrollbarSizeFactor), 1, drawnItemCount); // min: 1, max: drawnItemCount
			float scrollbarHalfSize = scrollbarSize / 2.f; // We try to draw half before 'i' and the other half after. If the result can not be cast to int, then
														   // before or after element 'i' needs to be one more [(int)2.5=2, (int)round(2.5)=3].
			int scrollbarTop_itemIndex = scrollbarOriginPos - (int)scrollbarHalfSize; // 0..drawnItemCount-1; If you round(halfSize) here, then you may not round by bottomPos - see halfSize init.
			int scrollbarBottom_itemIndex = scrollbarOriginPos + (int)scrollbarHalfSize; // 0..drawnItemCount-1
			if (scrollbarTop_itemIndex < 0) {
				scrollbarBottom_itemIndex += abs(scrollbarTop_itemIndex);
				scrollbarTop_itemIndex = 0;
			}
			else if (scrollbarBottom_itemIndex > drawnItemCount - 1) {
				scrollbarTop_itemIndex += (drawnItemCount - 1) - scrollbarBottom_itemIndex; // note += because the right value is negative.
				scrollbarBottom_itemIndex = drawnItemCount - 1;
			}
			assert(scrollbarTop_itemIndex >= 0 && scrollbarBottom_itemIndex <= drawnItemCount - 1);

			if (i >= startDrawIndex + scrollbarTop_itemIndex && i <= startDrawIndex + scrollbarBottom_itemIndex) {
				int boxRightPos = posX + (getDrawSize() - 2);
				int offset = boxRightPos - core::console::getCursorPos().x;
				std::cout << std::string(offset, ' ') << core::ColoredStr(" "s + uc::fullBlock, Color::Aqua);
			}
			std::cout << core::endl();
		}
	}
	std::cout.flush();
	drawBorder(getDrawSize(), false, posX);
}

intern void drawBorder(int size, bool isTop, int posX)
{
	std::string border = "";
	for (int i = 0; i < size - 2; ++i) { // size-2 because size includes " ^" / " v"
		border += isTop ? "_" : core::uc::overline;
	}
	std::cout << std::string(posX, ' ') << core::ColoredStr(border, core::Color::Gray) << " "
		<< (isTop ? core::uc::modifierLetterUpArrowhead : core::uc::modifierLetterDownArrowhead)
		<< core::endl();
}

void core::ScrollableList::onConsoleResize()
{
	posX = core::tab::size; // one tab from the left (8 chars)
	if (hasFlag(Options::DrawCentered, options)) {
		posX = console::getCharCount().x / 2.f - getDrawSize() / 2.f;
	}
}

void core::ScrollableList::clear()
{
	list.clear();
}

void core::ScrollableList::push_back(std::string item)
{
	list.push_back(item);
}

void core::ScrollableList::select(int index)
{
	selected = index;
}

void core::ScrollableList::selectHoveredItem()
{
	assert(hover != NOINDEX);
	selected = hover;
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

void core::ScrollableList::scrollToTop()
{
	hover = 0;
	startDrawIndex = 0;
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
	assert(selected != NOINDEX);
	return list.at(selected);
}

size_t core::ScrollableList::getHoverIndex()
{
	assert(hover != NOINDEX);
	return hover;
}

std::string core::ScrollableList::getHover()
{
	assert(hover != NOINDEX);
	return list.at(hover);
}

int core::ScrollableList::getDrawSize() const
{
	// Border length:
	return maxDrawnItemNameLength + 3 + 5 + 2; // +3: "...", +5: "300. ", +2 " ^"
}

int core::ScrollableList::getPosX() const
{
	return posX;
}

bool core::ScrollableList::isTrappedOnTop()
{
	return isTrappedOnTop_;
}

bool core::ScrollableList::isTrappedOnBottom()
{
	return isTrappedOnBottom_;
}