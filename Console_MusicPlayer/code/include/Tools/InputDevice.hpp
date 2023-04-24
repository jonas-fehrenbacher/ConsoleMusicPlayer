#pragma once

#include <vector>
#include <functional>

namespace core::inputDevice
{
	enum class MouseWheelScroll
	{
		Up = 0,
		Down
	};

	/* key codes: https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes */
	void init(std::function<void()> terminateApp);
	void terminate();
	void update();
	/* up:0; down:1 */
	std::vector<MouseWheelScroll> getMouseWheelScrollEvents();
	bool isKeyPressed(int key, bool ignoreLock = false);
	void lock(bool lock);
	bool isLocked();
}