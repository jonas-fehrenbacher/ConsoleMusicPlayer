#include "core/DrawableList.hpp"
#include "core/Console.hpp"
#include "core/SmallTools.hpp"
#include "core/InputDevice.hpp"
#include "core/Profiler.hpp"
#include <sstream>
#include <Windows.h>
#include <algorithm>
#include <cassert>
#include <iostream>

const size_t core::DrawableList::NOINDEX = -1;

void core::DrawableList::init(InitInfo info)
{
	this->name = info.name;
	this->sizeInside = info.sizeInside;
	this->hover = info.hover;
	this->style = info.style;
	this->options = info.options;
	this->spaceBetweenColumns = info.spaceBetweenColumns;
	this->columnLayout = info.columnLayout;
	selected = NOINDEX;
	drawnItemsSelectionPos = 0;
	isTrappedOnTop_ = false;
	isTrappedOnBottom_ = false;
	startDrawIndex = 0;
	hasFocus_ = true;
	paddingX = 2;

	if (columnLayout.empty()) {
		// Set number column:
		Column column;
		column.length            = Column::LARGEST_ITEM;
		column.color             = Color::White;
		column.isVisible         = true;
		column.hasEmptySpace     = false;
		column.isLengthInPercent = false;
		columnLayout.insert(columnLayout.begin(), column);

		// set default layout - expects only one column.
		column.length            = 0;
		column.color             = Color::White;
		column.isVisible         = true;
		column.hasEmptySpace     = true;
		column.isLengthInPercent = false;
		columnLayout.push_back(column);
	}

	for (auto& column : columnLayout) {
		assert(!column.isLengthInPercent || (column.isLengthInPercent && column.length != Column::LARGEST_ITEM));
	}

	onConsoleResize(); // calls calcColumnRawLength();
}

void core::DrawableList::terminate()
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
	hasFocus_ = true;
}

void core::DrawableList::update()
{
	if (!hasFocus_) {
		return;
	}

	// TODO: initList every few seconds... maybe in another thread.
}

void core::DrawableList::handleEvent()
{
	if (!hasFocus_) {
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

void core::DrawableList::move(bool up)
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

			if (startDrawIndex > 0 && hover < list.size() - sizeInside.y && drawnItemsSelectionPos == 0)
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

			if (startDrawIndex < list.size() - sizeInside.y && hover >= sizeInside.y && drawnItemsSelectionPos == sizeInside.y - 1)
				++startDrawIndex;
			if (drawnItemsSelectionPos < sizeInside.y - 1)
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
			if (startDrawIndex < list.size() - sizeInside.y)
				++startDrawIndex;
		}
	}
}

void core::DrawableList::draw()
{
	PROFILE_FUNC

	if (isFirstDraw) {
		// Make sure everything is updated:
		calcColumnRawLength();
		isFirstDraw = false;
	}

	drawBorder(true);
	//std::cout << core::endl();

	if (sizeInside.y <= 0) {
		// do nothing (but can cause an error if used bellow.)
	}
	else if (list.empty()) {
		std::cout << std::string(posX, ' ') << "Nothing found!" << core::endl();
	}
	else {
		///////////////////////////////////////////////////////////////////////////////
		// Set column sizes
		///////////////////////////////////////////////////////////////////////////////
		

		// Prepare drawing empty rows:
		if (hasFlag(Options::YSizeFitItemCount, options) && list.size() < sizeInside.y) {
			// TODO
		}

		// Output all rows:
		core::Text columnText("");
		std::stringstream ss;
		bool colorState = true;
		int drawnItemCount = list.size() < sizeInside.y ? list.size() : sizeInside.y;
		for (size_t i = startDrawIndex; i < startDrawIndex + drawnItemCount; ++i, colorState = !colorState) 
		{
			// Draw left border:
			std::cout << std::string(posX, ' ') << core::Text(core::uc::boxDrawingsLightVertical, style.border);

			///////////////////////////////////////////////////////////////////////////////
			// Output row text
			///////////////////////////////////////////////////////////////////////////////
			// Find last column index:
			// The last column is not neccessary 'columnLayout.size()-1', because a column is only drawn if its _rawLength is greater than 0.
			// Needed if user specifies a column with zero size - could cause an error without 'endColumnIndex'.
			int endColumnIndex = -1;
			for (auto& column : columnLayout) {
				if (column.isVisible && column._rawLength > 0)
					++endColumnIndex;
			}
			list[i].insert(list[i].begin(), std::to_string(i + 1)); // because first column is for item numbers and thats not inside ScrollableList::list - its kinda virtual.
			for (int j = 0; j < (int)columnLayout.size(); ++j)
			{
				if (!columnLayout[j].isVisible) {
					continue;
				}

				// (1) Set column text:
				ss.str("");
				if (j == 0) {
					// ..in first column the item number is displayed
					ss << std::right << std::setw(columnLayout[j]._rawLength) << list[i][j];
				}
				else 
				{
					std::string str = list[i][j].substr(0, columnLayout[j]._rawLength); // core::toStr(core::toWStr(list[i][j]).substr(0, columnLayout[j]._rawLength));
					
					if (str.length() >= 2 && list[i][j].length() > columnLayout[j]._rawLength) {
						str.replace(str.length() - 2, 2, "..");
					}
					else if (list[i][j].length() < columnLayout[j]._rawLength) {
						// ..fill left over space, so column aligns nicely
						// Attention - just let it be and do it bellow.
						// The string length is unknown, because there are utf8 strings which return a wrong size like "이루마". Its 3, but 6 on the console.
						// Using std::wstring_convert or what ever, it is not possible to get the string length from cout, so I could actually get the length from
						// the std::cout buffer itself by outputting it and calculate the size with the console cursor position. Afterwards I could reset the
						// cursor position and overwrite what I draw. Unfortunately, this is a bad idea because (1) printing to the console is slow, (2) 
						// the window flashes (even when I immediatelly overdraw it - it flickers). The best solution is to just set this offset bellow after
						// I outputted it regularly.
					}
					ss << str;
				}
				columnText = ss.str();

				// (2) Set column text color:
				{
					const int LAST_DRAWN_ITEM_INDEX = startDrawIndex + (sizeInside.y - 1);
					// TODO: can I not delete isFirstItem and isLastDrawn?
					bool isFirstDrawn = i == startDrawIndex;
					bool isFirstItem = i == 0;
					bool isLastDrawn = i == LAST_DRAWN_ITEM_INDEX;
					bool isLastItem = i == list.size() - 1;
					if (((isFirstDrawn && !isFirstItem) || (isLastDrawn && !isLastItem)) && list.size() > sizeInside.y) {
						// ...drawn item is at the top or bottom, but you can scroll higher or lower.
						columnText.fgcolor = style.borderItem;
						columnText.bgcolor = core::Color::None;
					}
					else {
						columnText.fgcolor = columnLayout[j].color; // colorState ? Color::White : Color::Bright_White;
						columnText.bgcolor = core::Color::None;
					}
					if (hasFocus_ && hasFlag(Options::SelectionMode, options) && i == hover) {
						assert(hover != NOINDEX);
						columnText.fgcolor = core::Color::Black;
						columnText.bgcolor = style.hover; // Light_Green, Light_Aqua; Light_Yellow, Bright_White
					}
					else if (hasFlag(Options::SelectionMode, options) && i == selected) {
						columnText.fgcolor = core::Color::Black;
						columnText.bgcolor = style.selected;
					}
				}

				// (3) Output:
				if (j == 0 && paddingX > 0) {
					// ..is before first column
					std::cout 
						// << std::string(1, ' ') // If you want padding with the selection background
						<< core::Text(std::string(paddingX, ' '), core::Color::None, columnText.bgcolor);
				}
				int startDrawPosX = core::console::getCursorPos().x;
				std::cout << columnText;
				int columnTextLength = core::console::getCursorPos().x - startDrawPosX; // columnText.str.length() can not return the size which is actually drawn to the console (some unicode chars have a additional white space - see notice on top, test it yourself: "이루마"). 
				std::cout << core::Text(std::string(columnLayout[j]._rawLength - columnTextLength, ' '), core::Color::None, columnText.bgcolor);
				if (j < endColumnIndex) {
					// ..is not last column
					std::cout << core::Text(std::string(spaceBetweenColumns, ' '), core::Color::None, columnText.bgcolor);
				}
			}
			list[i].erase(list[i].begin());

			// Padding:
			// You could just do: 'std::cout << std::string(paddingX, ' ');', but that below is better for debugging.
			int boxRightPos = posX + (getDrawSize() - 2);
			int offset = boxRightPos - core::console::getCursorPos().x;
			std::cout << core::Text(std::string(offset, ' '), core::Color::None, columnText.bgcolor);
			std::cout << " ";
			//assert(offset == paddingX);

			///////////////////////////////////////////////////////////////////////////////
			// Output row scrollbar
			///////////////////////////////////////////////////////////////////////////////
			// - Important: Unfortunately, std::setw() does not work for the scrollbar, because the music names have unicode characters and
			//   std::stringstream does not work properly with that (using std::wstringstream is to troublesome).
			// - Calculation:
			//   1. scrollbarOriginPos: Scrollbar position if it would be 1 character in size (is the center on a larger scrollbar).
			//   2. scrollbarSize: How large the scrollbar needs to be.
			//   3. scrollbarTop_itemIndex, scrollbarBottom_itemIndex: Start draw position and end draw position on the y axis.
			int middleDrawnItemIndex = round(startDrawIndex + sizeInside.y / 2.f); // using 'hover' is not that good if there are few items.
			float scrollbarPosFactor = middleDrawnItemIndex / (float)list.size(); // 0..1
			float scrollbarOriginPosRaw = (drawnItemCount - 1) * scrollbarPosFactor; // 0..drawnItemCount-1; Position if scrollbarSize==1
			int scrollbarOriginPos = (startDrawIndex + scrollbarOriginPosRaw) > list.size() / 2.f ? round(scrollbarOriginPosRaw) : scrollbarOriginPosRaw;
			// (sizeInside.y-1): This is neccessray, because we want 'sizeInside.y' positions (0..MDI-1) and not 'sizeInside.y+1' positions.
			float scrollbarSizeFactor = drawnItemCount / (float)list.size();
			int scrollbarSize = std::clamp((int)round(sizeInside.y * scrollbarSizeFactor), 1, drawnItemCount); // min: 1, max: drawnItemCount
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
				if (scrollbarTop_itemIndex < 0)
					scrollbarTop_itemIndex = 0; // TODO: this may not be triggered, but is..
				scrollbarBottom_itemIndex = drawnItemCount - 1;
			}
			assert(scrollbarTop_itemIndex >= 0 && scrollbarBottom_itemIndex <= drawnItemCount - 1);

			if (i >= startDrawIndex + scrollbarTop_itemIndex && i <= startDrawIndex + scrollbarBottom_itemIndex) {
				
				std::cout << core::Text(" "s, core::Color::Black, style.scrollbar);
			}
			else {
				std::cout << core::Text(" "s, core::Color::Black, style.scrollbarEmptySpace);
			}
			std::cout << core::endl();
		}
	}
	std::cout.flush();
	drawBorder(false);
}

int core::DrawableList::getItemNumberDrawSize() const
{
	return std::to_string(list.size()).length();
}

void core::DrawableList::drawBorder(bool isTop) const
{
	std::string border = "";
	std::string title = isTop ? " " + name + " " : "";
	for (int i = 0, borderLength = border.length(); i < (getDrawSize() - 4) - (int)title.length(); ++i) { // size-2 because size includes " ^" / " v"; (int)title.length() is required otherwise I could enter a endless loop.
		border +=  core::uc::boxDrawingsLightHorizontal;
	}
	std::cout << std::string(posX, ' ') 
		<< core::Text(std::string(isTop ? core::uc::boxDrawingsLightDownAndRight : core::uc::boxDrawingsLightUpAndRight) + core::uc::boxDrawingsLightHorizontal, style.border)
		<< core::Text(title, style.title)
		<< core::Text(border, style.border) 
		<< " "
		<< core::Text(isTop ? core::uc::blackUpPointingTriangle : core::uc::blackDownPointingTriangle, style.scrollbarArrow)
		<< core::endl();
}

void core::DrawableList::onConsoleResize()
{
	posX = 1;
	if (hasFlag(Options::DrawCentered, options)) {
		posX = console::getCharCount().x / 2.f - getDrawSize() / 2.f;
	}

	if (hasFlag(Options::DrawFullX, options)) {
		// ..to draw over the full x axis we draw as much as possible
		sizeInside.x = getDrawSize() - 2; // -2 for the border
	}

	calcColumnRawLength(); // call this after sizeInside is updated
}

void core::DrawableList::clear()
{
	list.clear();
}

void core::DrawableList::push_back(Row item)
{
	list.push_back(item);
}

void core::DrawableList::calcColumnRawLength()
{
	// Set LARGEST_ITEM:
	// column should be as large as its largest item.
	for (int i = 0; i < columnLayout.size(); ++i) {
		if (!columnLayout[i].isVisible) {
			continue;
		}

		if (columnLayout[i].length == Column::LARGEST_ITEM)
		{
			if (i == 0) {
				// ..item number column is a virtual column and the first.
				columnLayout[i]._rawLength = getItemNumberDrawSize();
			}
			else {
				int maxLength = 0;
				for (const Row& row : list) {
					maxLength = row[i - 1].length() > maxLength ? row[i - 1].length() : maxLength;
					// -1 because 'row' does not have the virtual column
				}

				columnLayout[i]._rawLength = maxLength;
			}
		}
	}
	// Calculate free row space (for dynamic sizes):
	int freeRowSpace = 0;
	{
		int fixRowSize = 0;
		for (auto& column : columnLayout) {
			if (!column.isVisible) {
				continue;
			}

			if (!column.isLengthInPercent) {
				if (column.length == Column::LARGEST_ITEM) {
					fixRowSize += column._rawLength; // is already set at the top
				}
				else {
					fixRowSize += column.length;
				}
			}
			fixRowSize += spaceBetweenColumns;
		}
		fixRowSize -= spaceBetweenColumns;
		fixRowSize += (paddingX * 2);
		assert(fixRowSize <= sizeInside.x);
		freeRowSpace = sizeInside.x - fixRowSize;
	}
	// Set column sizes:
	{
		int _freeRowSpace = freeRowSpace;
		for (auto& column : columnLayout) {
			if (!column.isVisible) {
				continue;
			}

			if (column.isLengthInPercent) {
				column._rawLength = (column.length / 100.f) * freeRowSpace; // W = p% * G
				_freeRowSpace -= column._rawLength;
			}
			else if (column.length != Column::LARGEST_ITEM) {
				column._rawLength = column.length;
			}
		}
		freeRowSpace = _freeRowSpace; // do this for the next step.
	}
	// Apply free space:
	{
		int columnWithEmptySpaceCount = 0; // How many columns want to have some of the still free space?
		int _freeRowSpace = freeRowSpace;
		for (auto& column : columnLayout) {
			if (!column.isVisible) {
				continue;
			}

			if (column.hasEmptySpace) {
				++columnWithEmptySpaceCount;
			}
		}
		for (int i = 0; i < columnLayout.size(); ++i) {
			if (!columnLayout[i].isVisible) {
				continue;
			}

			if (columnLayout[i].hasEmptySpace) {
				columnLayout[i]._rawLength += freeRowSpace / (float)columnWithEmptySpaceCount;
				_freeRowSpace -= freeRowSpace / (float)columnWithEmptySpaceCount;
			}
		}
		assert(_freeRowSpace == 0);

		// Debug:
		// Do I fill the whole row?
		int totalRowSize = 0;
		for (auto& column : columnLayout) {
			if (!column.isVisible) {
				continue;
			}

			totalRowSize += column._rawLength;
			totalRowSize += spaceBetweenColumns;
		}
		totalRowSize -= spaceBetweenColumns;
		totalRowSize += (paddingX * 2);
		assert(totalRowSize == sizeInside.x);
	}
}

void core::DrawableList::select(int index)
{
	selected = index;
}

void core::DrawableList::selectHoveredItem()
{
	assert(hover != NOINDEX);
	selected = hover;
}

void core::DrawableList::loseFocus()
{
	hasFocus_ = false;
	isTrappedOnBottom_ = false;
	isTrappedOnTop_ = false;
}

void core::DrawableList::gainFocus()
{
	hasFocus_ = true;
}

void core::DrawableList::scrollToTop()
{
	hover = 0;
	startDrawIndex = 0;
}

size_t core::DrawableList::size()
{
	return list.size();
}

size_t core::DrawableList::getSelectedIndex()
{
	return selected;
}

core::DrawableList::Row core::DrawableList::getSelected()
{
	assert(selected != NOINDEX);
	return list.at(selected);
}

size_t core::DrawableList::getHoverIndex()
{
	assert(hover != NOINDEX);
	return hover;
}

core::DrawableList::Row core::DrawableList::getHover()
{
	assert(hover != NOINDEX);
	return list.at(hover);
}

int core::DrawableList::getDrawSize() const
{
	// Border length:
	int size = 0;
	if (hasFlag(Options::DrawCentered, options) || !hasFlag(Options::DrawFullX, options)) {
		size = sizeInside.x + 2; // "|" + sizeInside.x + "^"
	}
	else if (hasFlag(Options::DrawFullX, options)) {
		size = core::console::getCharCount().x - 3; // try to draw as much as possible (-3 because scrollbar needs -2 and left border -1)
	}

	return size;
}

int core::DrawableList::getPosX() const
{
	return posX;
}

bool core::DrawableList::isTrappedOnTop() const
{
	return isTrappedOnTop_;
}

bool core::DrawableList::isTrappedOnBottom() const
{
	return isTrappedOnBottom_;
}

bool core::DrawableList::hasFocus() const
{
	return hasFocus_;
}