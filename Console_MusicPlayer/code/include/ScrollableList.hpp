#pragma once

#include <vector>
#include <string>
class App;

namespace core
{
	class ScrollableList
	{
	public:
		void init(App* app, bool selectionMode = true, int maxDrawnItems = 15, int maxDrawnItemNameLength = 60, size_t selected = 0);
		void terminate();
		void update();
		void handleEvent();
		void draw();

		void clear();
		void push_back(std::string item);
		/* No event handling and no update(). draw() is active, but selection will be ignored. Resets isTrapped flags. */
		void loseFocus();
		void gainFocus();
		size_t size();
		size_t getSelectedIndex();
		std::string getSelected();
		/* User is on the top and scrolls up, but can not leave the list. */
		bool isTrappedOnTop();
		/* User is on the bottom and scrolls down, but can not leave the list. */
		bool isTrappedOnBottom();
	private:
		App*                     app;
		std::vector<std::string> list;
		size_t                   selected;
		int                      maxDrawnItems;
		int                      maxDrawnItemNameLength;
		size_t                   startDrawIndex;
		bool                     selectionMode; // you can select items in this mode, not just scroll.
		int                      drawnItemsSelectionPos; // controls startDrawIndex
		bool                     isTrappedOnTop_;
		bool                     isTrappedOnBottom_;
		bool                     hasFocus;

		void move(bool up);
	};
}