#pragma once

#include "MessageBus.hpp"
#include <vector>
#include <functional>

/**
 * Note SDL2 can not be used, because it requires a window which is on focus, so it does not work with console applications.
 */

namespace core::inputDevice
{
	enum class Key {
		Unknown = -1, 
		A = 0, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z, 
		Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9, 
		Escape, LControl, LShift, LAlt, LSystem, RControl, RShift, RAlt, RSystem, Menu, 
		LBracket, RBracket, Semicolon, Comma, Period, Quote, Slash, Backslash, Tilde, Equal, Plus, Minus/*Hyphen*/, Space, Enter, Backspace, Tab,
		PageUp, PageDown, End, Home, Insert, Delete,
		Left, Right, Up, Down,
		NumpadAdd, NumpadSubtract, NumpadMultiply, NumpadDivide, NumpadSeperator, NumpadDecimal, 
		Numpad0, Numpad1, Numpad2, Numpad3, Numpad4, Numpad5, Numpad6, Numpad7, Numpad8, Numpad9, 
		F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, F13, F14, F15, 
		Pause,
		MediaNextTrack, MediaPrevTrack, MediaStop, MediaPlayPause,
		KeyCount
	};

	enum class MouseWheelScroll
	{
		Up = 0,
		Down
	};

	void init(std::function<void()> terminateApp, core::MessageBus* messageBus);
	void terminate();
	void update();
	void lock(bool lock);
	/** Get mouse wheel scroll state: up:0; down:1 */
	std::vector<MouseWheelScroll> getMouseWheelScrollEvents();
	/** Clears state of the specified key. */
	void clearKeyState(Key key);
	/** Only triggered when press event is first registered. */
	bool isKeyPressed(Key key, bool ignoreLock = false);
	/** Key is registered as held down key after ~200ms of pressing it, but this time is not elapsed, yet. */
	bool isKeyWaitingForHeldDownRegistration(Key key, bool ignoreLock = false);
	/** Key is held down (triggered after ~200ms). */
	bool isKeyHeldDown(Key key, bool ignoreLock = false);
	/** Key is released. */
	bool isKeyReleased(Key key, bool ignoreLock = false);
	bool isLocked();
}