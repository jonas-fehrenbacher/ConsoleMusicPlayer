#pragma once

#include "core/MessageBus.hpp"

enum Message
{
	MenuState_EnteredPlaylist = core::MessageID::CUSTOM_MESSAGE,
	Playlist_back,
	Music_Play,
	Music_Pause,
	Music_Stop
};