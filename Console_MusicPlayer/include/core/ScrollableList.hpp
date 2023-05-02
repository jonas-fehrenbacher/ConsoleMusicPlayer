#pragma once

#include <vector>
#include <string>
#include "Console.hpp"

namespace core
{
	/* UI element */
	class ScrollableList
	{
	public:
		enum Options
		{
			None          = 0,
			SelectionMode = 1 << 0, // you can select items in this mode, not just scroll.
			ArrowInput    = 1 << 1, // You can scroll and use arrows
			DrawCentered  = 1 << 2, // Box is drawed in the center of the console instead of one tab from the left; Either this or DrawFullX can be active
			DrawFullX     = 1 << 3  // Draws on x axis completly; Either this or DrawCentered can be active
		};

		struct Style
		{
			core::Color border;
			core::Color title;
			core::Color scrollbarArrow;
			core::Color scrollbar;
			core::Color scrollbarEmptySpace;
			core::Color item;
			core::Color borderItem;
			core::Color selected;
			core::Color hover;
		};

		Style style;

		void init(Options options, Style style, std::string name, int maxDrawnItems = 15, int maxDrawnItemNameLength = 60, size_t hover = 0);
		void terminate();
		void update();
		void handleEvent();
		void draw();

		void select(int index);
		/* ScrollableList does not selected elements, the user has to do this. */
		void selectHoveredItem();
		void clear();
		void push_back(std::string item);
		void onConsoleResize();
		/* No event handling and no update(). draw() is active, but selection will be ignored. Resets isTrapped flags. */
		void loseFocus();
		void gainFocus();
		void scrollToTop();
		size_t size();
		/** returns -1 if nothing is selected */
		size_t getSelectedIndex();
		std::string getSelected();
		size_t getHoverIndex();
		std::string getHover();
		/* return length of the drawn box in character column count. Note that this ignores the tab and Scrollbar - its just the border. */
		int getDrawSize() const;
		int getPosX() const;
		/* User is on the top and scrolls up, but can not leave the list. */
		bool isTrappedOnTop();
		/* User is on the bottom and scrolls down, but can not leave the list. */
		bool isTrappedOnBottom();
	private:
		std::vector<std::string> list;
		size_t                   selected; //< item index which is "selected"
		size_t                   hover; //< item index over which you "hover"
		int                      maxDrawnItems;
		int                      maxDrawnItemNameLength;
		size_t                   startDrawIndex;
		Options                  options;
		int                      drawnItemsSelectionPos; // controls startDrawIndex
		bool                     isTrappedOnTop_;
		bool                     isTrappedOnBottom_;
		bool                     hasFocus;
		int                      posX; // draw position on the x axis (in characters)
		std::string              name;

		void move(bool up);
		int getNonTitleElementsLength() const;
		void drawBorder(bool isTop) const;
	};
}