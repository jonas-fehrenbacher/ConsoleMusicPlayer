#include "core/InputDevice.hpp"
#include "core/Timer.hpp"
#include "core/SmallTools.hpp"
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <iostream>
#include <array>

namespace core::inputDevice
{
    struct PKey // physical key
    {
        Key   type;
        Timer holdTimer; //< timer to check if key is held down. press: 0s -> isHeldDown: >100ms; release: 0s

        bool isPressEventStart; //< only triggered when press event is first registered.
        bool isWaitingForHeldDownRegistration; //< Key is registered as held down key after ~200ms of pressing it, but this time is not elapsed, yet.
        bool isHeldDown; //< key is held down (triggered after ~200ms).
        bool isReleased; //< key is released
    };
    std::array<PKey, (size_t)Key::KeyCount> keyboardState; //< holds the current keyboard state
    std::array<int, (size_t)Key::KeyCount> windowsKeyMap; //< Maps ::core::Key to windows api key: winKey = windowsKeyMap[Key::A].

	bool isLocked_ = false;
	core::Timer cooldownKey;
    std::vector<MouseWheelScroll> mouseWheelEvents; // bool-vector is no standard stl container, so I use short.
    std::function<void()> terminateApp;
    HANDLE hStdin;
    DWORD fdwSaveOldMode;
    core::MessageBus* messageBus;

    intern void _terminate();
}

intern BOOL onConsoleEvent(DWORD event);
void core::inputDevice::init(std::function<void()> terminateApp, core::MessageBus* messageBus)
{
    core::inputDevice::terminateApp = terminateApp;
    core::inputDevice::messageBus = messageBus;
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

    ///////////////////////////////////////////////////////////////////////////////
    // Init key state
    ///////////////////////////////////////////////////////////////////////////////
    for (int key = 0; key < (int)Key::KeyCount; ++key) {
        keyboardState[key].type = (Key)key;
        keyboardState[key].holdTimer.stop();
    }

    ///////////////////////////////////////////////////////////////////////////////
    // Init windows key map
    ///////////////////////////////////////////////////////////////////////////////
    /* Key codes: https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes */
    windowsKeyMap[(int)Key::A]               = 'A';
    windowsKeyMap[(int)Key::B]               = 'B';
    windowsKeyMap[(int)Key::C]               = 'C';
    windowsKeyMap[(int)Key::D]               = 'D';
    windowsKeyMap[(int)Key::E]               = 'E';
    windowsKeyMap[(int)Key::F]               = 'F';
    windowsKeyMap[(int)Key::G]               = 'G';
    windowsKeyMap[(int)Key::H]               = 'H';
    windowsKeyMap[(int)Key::I]               = 'I';
    windowsKeyMap[(int)Key::J]               = 'J';
    windowsKeyMap[(int)Key::K]               = 'K';
    windowsKeyMap[(int)Key::L]               = 'L';
    windowsKeyMap[(int)Key::M]               = 'M';
    windowsKeyMap[(int)Key::N]               = 'N';
    windowsKeyMap[(int)Key::O]               = 'O';
    windowsKeyMap[(int)Key::P]               = 'P';
    windowsKeyMap[(int)Key::Q]               = 'Q';
    windowsKeyMap[(int)Key::R]               = 'R';
    windowsKeyMap[(int)Key::S]               = 'S';
    windowsKeyMap[(int)Key::T]               = 'T';
    windowsKeyMap[(int)Key::U]               = 'U';
    windowsKeyMap[(int)Key::V]               = 'V';
    windowsKeyMap[(int)Key::W]               = 'W';
    windowsKeyMap[(int)Key::X]               = 'X';
    windowsKeyMap[(int)Key::Y]               = 'Y';
    windowsKeyMap[(int)Key::Z]               = 'Z';
    windowsKeyMap[(int)Key::Num0]            = '0';
    windowsKeyMap[(int)Key::Num1]            = '1';
    windowsKeyMap[(int)Key::Num2]            = '2';
    windowsKeyMap[(int)Key::Num3]            = '3';
    windowsKeyMap[(int)Key::Num4]            = '4';
    windowsKeyMap[(int)Key::Num5]            = '5';
    windowsKeyMap[(int)Key::Num6]            = '6';
    windowsKeyMap[(int)Key::Num7]            = '7';
    windowsKeyMap[(int)Key::Num8]            = '8';
    windowsKeyMap[(int)Key::Num9]            = '9';
    windowsKeyMap[(int)Key::Escape]          = VK_ESCAPE;
    windowsKeyMap[(int)Key::LControl]        = VK_LCONTROL;
    windowsKeyMap[(int)Key::LShift]          = VK_LSHIFT;
    windowsKeyMap[(int)Key::LAlt]            = VK_LMENU;
    windowsKeyMap[(int)Key::LSystem]         = VK_LWIN;
    windowsKeyMap[(int)Key::RControl]        = VK_RCONTROL;
    windowsKeyMap[(int)Key::RShift]          = VK_RSHIFT;
    windowsKeyMap[(int)Key::RAlt]            = VK_RMENU;
    windowsKeyMap[(int)Key::RSystem]         = VK_RWIN;
    windowsKeyMap[(int)Key::Menu]            = VK_MENU; // right key?
    windowsKeyMap[(int)Key::LBracket]        = VK_OEM_4;
    windowsKeyMap[(int)Key::RBracket]        = VK_OEM_6;
    windowsKeyMap[(int)Key::Semicolon]       = VK_OEM_1;
    windowsKeyMap[(int)Key::Comma]           = VK_OEM_COMMA;
    windowsKeyMap[(int)Key::Period]          = VK_OEM_PERIOD;
    windowsKeyMap[(int)Key::Quote]           = VK_OEM_7;
    windowsKeyMap[(int)Key::Slash]           = VK_OEM_2; // or VK_DIVIDE?
    windowsKeyMap[(int)Key::Backslash]       = VK_OEM_5;
    windowsKeyMap[(int)Key::Tilde]           = VK_OEM_3;
    windowsKeyMap[(int)Key::Equal]           = VK_OEM_NEC_EQUAL; // '=' key on numpad
    windowsKeyMap[(int)Key::Plus]            = VK_OEM_PLUS;
    windowsKeyMap[(int)Key::Minus]           = VK_OEM_MINUS;
    windowsKeyMap[(int)Key::Space]           = VK_SPACE;
    windowsKeyMap[(int)Key::Enter]           = VK_RETURN;
    windowsKeyMap[(int)Key::Backspace]       = VK_BACK;
    windowsKeyMap[(int)Key::Tab]             = VK_TAB;
    windowsKeyMap[(int)Key::PageUp]          = VK_PRIOR;
    windowsKeyMap[(int)Key::PageDown]        = VK_NEXT;
    windowsKeyMap[(int)Key::End]             = VK_END;
    windowsKeyMap[(int)Key::Home]            = VK_HOME;
    windowsKeyMap[(int)Key::Insert]          = VK_INSERT;
    windowsKeyMap[(int)Key::Delete]          = VK_DELETE;
    windowsKeyMap[(int)Key::Left]            = VK_LEFT;
    windowsKeyMap[(int)Key::Right]           = VK_RIGHT;
    windowsKeyMap[(int)Key::Up]              = VK_UP;
    windowsKeyMap[(int)Key::Down]            = VK_DOWN;
    windowsKeyMap[(int)Key::NumpadAdd]       = VK_ADD;
    windowsKeyMap[(int)Key::NumpadSubtract]  = VK_SUBTRACT;
    windowsKeyMap[(int)Key::NumpadMultiply]  = VK_MULTIPLY;
    windowsKeyMap[(int)Key::NumpadDivide]    = VK_DIVIDE;
    windowsKeyMap[(int)Key::NumpadSeperator] = VK_SEPARATOR;
    windowsKeyMap[(int)Key::NumpadDecimal]   = VK_DECIMAL;
    windowsKeyMap[(int)Key::Numpad0]         = VK_NUMPAD0;
    windowsKeyMap[(int)Key::Numpad1]         = VK_NUMPAD1;
    windowsKeyMap[(int)Key::Numpad2]         = VK_NUMPAD2;
    windowsKeyMap[(int)Key::Numpad3]         = VK_NUMPAD3;
    windowsKeyMap[(int)Key::Numpad4]         = VK_NUMPAD4;
    windowsKeyMap[(int)Key::Numpad5]         = VK_NUMPAD5;
    windowsKeyMap[(int)Key::Numpad6]         = VK_NUMPAD6;
    windowsKeyMap[(int)Key::Numpad7]         = VK_NUMPAD7;
    windowsKeyMap[(int)Key::Numpad8]         = VK_NUMPAD8;
    windowsKeyMap[(int)Key::Numpad9]         = VK_NUMPAD9;
    windowsKeyMap[(int)Key::F1]              = VK_F1;
    windowsKeyMap[(int)Key::F2]              = VK_F2;
    windowsKeyMap[(int)Key::F3]              = VK_F3;
    windowsKeyMap[(int)Key::F4]              = VK_F4;
    windowsKeyMap[(int)Key::F5]              = VK_F5;
    windowsKeyMap[(int)Key::F6]              = VK_F6;
    windowsKeyMap[(int)Key::F7]              = VK_F7;
    windowsKeyMap[(int)Key::F8]              = VK_F8;
    windowsKeyMap[(int)Key::F9]              = VK_F9;
    windowsKeyMap[(int)Key::F10]             = VK_F10;
    windowsKeyMap[(int)Key::F11]             = VK_F11;
    windowsKeyMap[(int)Key::F12]             = VK_F12;
    windowsKeyMap[(int)Key::F13]             = VK_F13;
    windowsKeyMap[(int)Key::F14]             = VK_F14;
    windowsKeyMap[(int)Key::F15]             = VK_F15;
    windowsKeyMap[(int)Key::Pause]           = VK_PAUSE;
    windowsKeyMap[(int)Key::MediaNextTrack]  = VK_MEDIA_NEXT_TRACK;
    windowsKeyMap[(int)Key::MediaPrevTrack]  = VK_MEDIA_PREV_TRACK;
    windowsKeyMap[(int)Key::MediaStop]       = VK_MEDIA_STOP;
    windowsKeyMap[(int)Key::MediaPlayPause]  = VK_MEDIA_PLAY_PAUSE;
}

intern BOOL onConsoleEvent(DWORD event) {

    switch (event)
    {
    case CTRL_C_EVENT:
    case CTRL_CLOSE_EVENT:
        core::inputDevice::_terminate();
        break;
    }

    return TRUE;
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
    ///////////////////////////////////////////////////////////////////////////////
    // Load mouse wheel scroll events & send resize event
    ///////////////////////////////////////////////////////////////////////////////
    // ReadConsoleInput() gets only events if the console is on focus!
    // See: https://iq.direct/blog/325-how-to-read-direct-mouse-and-keyboard-events-in-c-console-app.html
    std::vector<int> pressedKeys;
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
            case KEY_EVENT: 
                if (irInBuf[i].Event.KeyEvent.bKeyDown) {
                    // ..key is pressed
                    pressedKeys.push_back(irInBuf[i].Event.KeyEvent.wVirtualKeyCode);
                }
                break;
            case WINDOW_BUFFER_SIZE_EVENT: // scrn buf. resizing; irInBuf[i].Event.WindowBufferSizeEvent
                messageBus->send(MessageID::CONSOLE_RESIZE);
            break;
            //case FOCUS_EVENT: break; // disregard focus events 
            //case MENU_EVENT: break; // These events are used internally and should be ignored. See: https://learn.microsoft.com/en-us/windows/console/input-record-str
            //default: break;
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////////
    // Load key state
    ///////////////////////////////////////////////////////////////////////////////
    // GetAsyncKeyState(key) removes event after one call, so I need to store its state for one system iteration.
    // Furthermore, this allows to check if key is held down or released.
    /*
        VK = Virtual Key
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
    static Timer::Duration releaseDelay = 200ms;
    for (int key = 0; key < (int)Key::KeyCount; ++key) {
        bool isPressed = (GetAsyncKeyState(windowsKeyMap[key]) & 0x8000) || (GetKeyState(windowsKeyMap[key]) & 0x8000) || // get also GetKeyState events, because another program could "steel" them. At least if the console is on focus, I get all events.
            std::find(pressedKeys.begin(), pressedKeys.end(), windowsKeyMap[key]) != pressedKeys.end();
        // On a focused console there are key events which do not get triggered with GetKeyState(). Thats why I store key events 
        // from ReadConsoleInput() inside 'pressedKeys' and use it as well.
        // Quering the current key state is inaccurate. Maybe: If the handleEvents() function is not as often called (because draw function takes to long or something else). Using the windows event queue should be save.

        clearKeyState((Key)key);

        if (isPressed && keyboardState[key].holdTimer.isStopped()) {
            // start press event
            keyboardState[key].isPressEventStart = true;
            keyboardState[key].holdTimer.restart();
            continue;
        }
        if (isPressed && !keyboardState[key].holdTimer.isStopped()) {
            if (keyboardState[key].holdTimer.getElapsedTime() <= releaseDelay) {
                // key is theoretically held down, but maybe user wants to do a (quick) press.
                keyboardState[key].isWaitingForHeldDownRegistration = true;
                continue;
            }
            else {
                // key is held down
                keyboardState[key].isHeldDown = true;
                continue;
            }
        }
        if (!isPressed && !keyboardState[key].holdTimer.isStopped()) {
            // release event
            keyboardState[key].isReleased = true;
            keyboardState[key].holdTimer.stop();
            continue;
        }
    }
}

void core::inputDevice::lock(bool lock)
{
    isLocked_ = lock;
}

std::vector<core::inputDevice::MouseWheelScroll> core::inputDevice::getMouseWheelScrollEvents()
{
    return mouseWheelEvents;
}

void core::inputDevice::clearKeyState(Key key)
{
    keyboardState[(int)key].isPressEventStart                = false;
    keyboardState[(int)key].isWaitingForHeldDownRegistration = false;
    keyboardState[(int)key].isHeldDown                       = false;
    keyboardState[(int)key].isReleased                       = false;
}

bool core::inputDevice::isKeyPressed(Key key, bool ignoreLock /*= false*/)
{
    return (!isLocked_ || ignoreLock) && keyboardState[(int)key].isPressEventStart;
}

bool core::inputDevice::isKeyWaitingForHeldDownRegistration(Key key, bool ignoreLock /*= false*/)
{
    return (!isLocked_ || ignoreLock) && keyboardState[(int)key].isWaitingForHeldDownRegistration;
}

bool core::inputDevice::isKeyHeldDown(Key key, bool ignoreLock /*= false*/)
{
    return (!isLocked_ || ignoreLock) && keyboardState[(int)key].isHeldDown;
}

bool core::inputDevice::isKeyReleased(Key key, bool ignoreLock /*= false*/)
{
    return (!isLocked_ || ignoreLock) && keyboardState[(int)key].isReleased;
}

bool core::inputDevice::isLocked()
{
	return isLocked_;
}