#pragma once

namespace core::inputDevice
{
	/* key codes: https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes */
	bool isKeyPressed(int key, bool ignoreLock = false);
	void lock(bool lock);
	bool isLocked();
}