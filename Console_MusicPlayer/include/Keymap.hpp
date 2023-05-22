#pragma once

#include "core/InputDevice.hpp"
#include <array>
#include <string>

class Keymap
{
public:
	enum class Action
	{
		Exit,
		Back,
		PrevTrack,
		NextTrack,
		LockInput,
		SelectNavBar,
		PlayPause,
		Shuffle,
		IncreaseVolume,
		DecreaseVolume,
		Repeat,
		TrackSkipBackward,
		TrackSkipForward,
		KeyInfo,
		Select,
		
		Count
	};

	struct Key
	{
		core::inputDevice::Key key;
		std::string            symbol;
	};

	/** Load key map from data/keymap.properties. */
	void init();
	/** Get the key to a specific action. */
	Key get(Action action);
private:
	std::array<core::inputDevice::Key, (int)Action::Count>         data;
	std::array<std::string, (int)core::inputDevice::Key::KeyCount> symbolMap;
};