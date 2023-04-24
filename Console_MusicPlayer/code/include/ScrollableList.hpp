#pragma once

#include <vector>
#include <string>
class App;

namespace core
{
	class ScrollableList
	{
	public:
		void init(App* app, int maxDrawnItems = 10, int maxDrawnItemNameLength = 60, size_t selected = 0);
		void terminate();
		void update();
		void handleEvent();
		void draw();

		void clear();
		void push_back(std::string item);
		size_t size();
		size_t getSelectedIndex();
		std::string getSelected();
	private:
		App*                     app;
		std::vector<std::string> list;
		size_t                   selected;
		int                      maxDrawnItems;
		int                      maxDrawnItemNameLength;
		size_t                   startDrawIndex;

		void initList();
	};
}