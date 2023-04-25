#include "Tools/InputDevice.hpp"
#include "Timer/Timer.hpp"
#include "Tools/Tool.hpp"
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <iostream>

namespace core::inputDevice
{
	bool isLocked_ = false;
	core::Timer cooldownKey;
    std::vector<MouseWheelScroll> mouseWheelEvents; // bool-vector is no standard stl container, so I use short.
    std::function<void()> terminateApp;
    HANDLE hStdin;
    DWORD fdwSaveOldMode;

    intern void _terminate();
}

BOOL onConsoleEvent(DWORD event) {

    switch (event) 
    {
    case CTRL_C_EVENT:
    case CTRL_CLOSE_EVENT:
        core::inputDevice::_terminate();
        break;
    }

    return TRUE;
}

void core::inputDevice::init(std::function<void()> terminateApp)
{
    core::inputDevice::terminateApp = terminateApp;
    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdin == INVALID_HANDLE_VALUE)
        log("GetStdHandle");
    if (!GetConsoleMode(hStdin, &fdwSaveOldMode)) // Save the current input mode, to be restored on exit. 
        log("GetConsoleMode");

    // Enable the window and mouse input events:
    DWORD fdwMode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT | ENABLE_INSERT_MODE | ENABLE_EXTENDED_FLAGS;
    if (!SetConsoleMode(hStdin, fdwMode))
        log("SetConsoleMode");

    // Set console close event callback:
    if (!SetConsoleCtrlHandler(onConsoleEvent, TRUE)) {
        log("SetConsoleCtrlHandler failed!");
        __debugbreak();
    }
}

intern void core::inputDevice::_terminate()
{
    terminateApp(); // This should call inputDevice::terminate().
}

void core::inputDevice::terminate()
{
    SetConsoleMode(hStdin, fdwSaveOldMode); // Restore input mode on exit.
}

void core::inputDevice::update()
{
    // Load mouse wheel scroll events:
    // See: https://iq.direct/blog/325-how-to-read-direct-mouse-and-keyboard-events-in-c-console-app.html
    mouseWheelEvents.clear();
    INPUT_RECORD irInBuf[128];
    DWORD cNumRead;
    unsigned long eventCount;
    GetNumberOfConsoleInputEvents(hStdin, &eventCount);
    if (eventCount > 0)
    {
        // ReadConsoleInput() does not return until at least one input record has been read!
        // This behavior should be fixed with GetNumberOfConsoleInputEvents().
        // See: https://learn.microsoft.com/en-us/windows/console/readconsoleinput
        if (!ReadConsoleInput(
            hStdin,      // input buffer handle 
            irInBuf,     // buffer to read into 
            128,         // size of read buffer 
            &cNumRead)) // number of records read 
            log("ReadConsoleInput");

        // If you use a loop:
        // [ GetNumberOfConsoleInputEvents(,eventCount); ReadConsoleInput(,,,cNumRead); ]
        // Important: Do not write: 'eventCount -= cNumRead;', sometimes this results in an endless loop, because 'cNumRead' 
        // increases in this short amount of time, thus 'eventCount' as an unsigned will be really big.

        for (int i = 0; i < cNumRead; i++)
        {
            switch (irInBuf[i].EventType)
            {
            case MOUSE_EVENT: // mouse input 
                switch (irInBuf[i].Event.MouseEvent.dwEventFlags)
                {
                case MOUSE_WHEELED:
                    // If the high word of the dwButtonState member contains a positive value, the wheel was rotated forward, away from the user. Otherwise, the wheel was rotated backward, toward the user.
                    // See: https://learn.microsoft.com/en-us/windows/console/mouse-event-record-str
                    int delta = HIWORD(irInBuf[i].Event.MouseEvent.dwButtonState); // 120 up; 65416 down
                    // the hi order word might be negative, but WORD is unsigned, so
                    // we need some signed type of an appropriate size:
                    // See: https://stackoverflow.com/questions/53466611/c-identifying-x-buttons-scroll-wheel-directions
                    bool isDown = static_cast<std::make_signed_t<WORD>>(HIWORD(irInBuf[i].Event.MouseEvent.dwButtonState)) < 0; // up: 0; down: 1
                    mouseWheelEvents.push_back((MouseWheelScroll)isDown);
                break;
                //case 0:
                //    if (irInBuf[i].Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED) {} // printf("left button press \n");
                //    else if (irInBuf[i].Event.MouseEvent.dwButtonState == RIGHTMOST_BUTTON_PRESSED) {} // printf("right button press \n");
                //    else {} // printf("button press\n");
                //    break;
                //case DOUBLE_CLICK: break;
                //case MOUSE_HWHEELED: break;
                //case MOUSE_MOVED: break;
                //default: break;
                }
            break;
            //case KEY_EVENT: break; // keyboard input; irInBuf[i].Event.KeyEvent
            //case WINDOW_BUFFER_SIZE_EVENT: break; // scrn buf. resizing; irInBuf[i].Event.WindowBufferSizeEvent
            //case FOCUS_EVENT: break; // disregard focus events 
            //case MENU_EVENT: break; // These events are used internally and should be ignored. See: https://learn.microsoft.com/en-us/windows/console/input-record-str
            //default: break;
            }
        }
    }
}

std::vector<core::inputDevice::MouseWheelScroll> core::inputDevice::getMouseWheelScrollEvents()
{
    return mouseWheelEvents;
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