#pragma once

#include "MessageBus.hpp"
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
	void init(std::function<void()> terminateApp, core::MessageBus* messageBus);
	void terminate();
	void update();
	void lock(bool lock);
	/* up:0; down:1 */
	std::vector<MouseWheelScroll> getMouseWheelScrollEvents();
	bool isKeyPressed(int key, bool ignoreLock = false);
	bool isLocked();
}