#pragma once

#include <vector>
#include <string>
#include "Console.hpp"

namespace core
{
	/* UI element */
	class DrawableList
	{
	public:
		using Row = std::vector<std::string>; // list of columns

		enum Options
		{
			None              = 0,
			SelectionMode     = 1 << 0, // you can select items in this mode, not just scroll.
			ArrowInput        = 1 << 1, // You can scroll and use arrows
			DrawCentered      = 1 << 2, // Box is drawed in the center of the console instead of one tab from the left; Either this or DrawFullX can be active
			DrawFullX         = 1 << 3, // Draws on x axis completly; Either this or DrawCentered can be active
			YSizeFitItemCount = 1 << 4  // Not implemented yet; List is as large as its item count, maximum is InitInfo::sizeInside::y.
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

		/** By default there is at the beginning a column to display the item number. */
		struct Column
		{
			static constexpr int LARGEST_ITEM = -1; //< Column is as large as the largest item ('length = LARGEST_ITEM').

			int   length;
			Color color;
			bool  alignRight;        //< Aligns all column items to the right, which is only doable if length == LARGEST_ITEM.
			bool  isVisible;         //< Only visible columns are drawn. Useful if you want to use an filter or don't want to show all entries. If this changes 'calcColumnRawLength' needs to be called!
			bool  hasEmptySpace;     //< Coulmn gets all available empty space.
			bool  isLengthInPercent; //< Percentage of the still available empty space in the list. Otherwise length is in character count.
			int   _rawLength;        //< internal / private raw length (ignore this, its used by core)
		};

		struct InitInfo
		{
			Options             options; 
			Style               style; 
			std::string         name;                 //< Name of the list is displayed on the top.
			std::vector<Column> columnLayout;         //< The column layout - how should the list data be interpreted? (optional for one column)
			int                 spaceBetweenColumns;  //< Space between columns (ignored on one column)
			Vec2                sizeInside;           //< Size inside the list as column character count * row count. X axis is ignored if Option::DrawFullX is set. ScrollableList can be smaller if there are not enough items to fill all rows.
			size_t              hover;                //< At which item should you start scrolling?
		};

		static const size_t NOINDEX; // -1 stands for no index (see 'selected')
		Style style;

		void init(InitInfo info);
		void terminate();
		void update();
		void handleEvent();
		void draw();

		/** Use DrawableList::NOINDEX to select nothing. */
		void select(int index);
		/* ScrollableList does not selected elements, the user has to do this. */
		void selectHoveredItem();
		void clear();
		void push_back(Row item);
		void onConsoleResize();
		/* No event handling and no update(). draw() is active, but selection will be ignored. Resets isTrapped flags. */
		void loseFocus();
		void gainFocus();
		void scrollToTop();
		size_t size() const;
		/** returns -1 if nothing is selected */
		size_t getSelectedIndex() const;
		Row getSelected() const;
		size_t getHoverIndex() const;
		Row getHover() const;
		const std::vector<Row>& get() const;
		/* return length of the drawn box in character column count. Note that this ignores the tab and Scrollbar - its just the border. */
		int getDrawSize() const;
		int getPosX() const;
		/* User is on the top and scrolls up, but can not leave the list. */
		bool isTrappedOnTop() const;
		/* User is on the bottom and scrolls down, but can not leave the list. */
		bool isTrappedOnBottom() const;
		bool hasFocus() const;
	private:
		std::vector<Row>         list;
		size_t                   selected; //< item index which is "selected"
		size_t                   hover; //< item index over which you "hover"
		Vec2                     sizeInside; //< size inside the list (without border, scrollbar, ..) as column character count * row count.
		size_t                   startDrawIndex;
		Options                  options;
		int                      drawnItemsSelectionPos; // controls startDrawIndex
		bool                     isTrappedOnTop_;
		bool                     isTrappedOnBottom_;
		bool                     hasFocus_;
		int                      posX; // draw position on the x axis (in characters)
		std::string              name;
		int                      spaceBetweenColumns;
		std::vector<Column>      columnLayout;
		int                      paddingX; // Space between border and column (padding on x axis); Style!?
		bool                     isFirstDraw; // optional, but save

		void move(bool up);
		void drawBorder(bool isTop) const;
		int getItemNumberDrawSize() const;
		void calcColumnRawLength();
	};
}