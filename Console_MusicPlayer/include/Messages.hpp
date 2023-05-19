#pragma once

#include "core/MessageBus.hpp"

enum Message
{
	NAVBAR_SHORTCUT_TRIGGERED = core::MessageID::CUSTOM_MESSAGE, //< States should listen to this and set themselves off focus (halt events, stop draw hover, ...).
	NAVBAR_OPTION_SELECTED, //< App listens to this and changes state; userData: NavBar::Option
	NAVBAR_BACK //< States listen to this and set themselves on focus, again. User wants to resume old state - no hard reset like in 'NAVBAR_OPTION_SELECTED'.; userData: NavBar::Option (selected)
};