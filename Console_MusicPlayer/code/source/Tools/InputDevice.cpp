#include "Tools/InputDevice.hpp"
#include "Timer/Timer.hpp"
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace core::inputDevice
{
	bool isLocked_ = false;
	core::Timer cooldownKey;
}

bool core::inputDevice::isKeyPressed(int key, bool ignoreLock /*= false*/)
{
	/* 
		Key state functions:
		GetKeyState(): is based on a message queue and ignores keyboard input that was send to another program.
		GetAsyncKeyState(): immediate physical state of the keyboard
		See: https://learn.microsoft.com/en-us/windows/win32/learnwin32/keyboard-input
		Return / Flags:
		0x8000 (most significant bit): contains the bit flag that tests whether the key is currently pressed / down.
		0x1 (least significant bit): key was pressed after the previous call to GetAsyncKeyState (not relyable).
		Is 0 if the current desktop is not the active desktop.
		Works as well with mouse buttons.
		Although the least significant bit of the return value indicates whether the key has been pressed since the last query, 
		due to the preemptive multitasking nature of Windows, another application can call GetAsyncKeyState and receive the 
		"recently pressed" bit instead of your application.
		See: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getasynckeystate
	*/
	if (GetAsyncKeyState(key) & 0x8000 && cooldownKey.getElapsedTime() > 200ms && (!isLocked_ || ignoreLock))
	{
		cooldownKey.restart();
		return true;
	}
	return false;
}

void core::inputDevice::lock(bool lock)
{
	isLocked_ = lock;
}

bool core::inputDevice::isLocked()
{
	return isLocked_;
}