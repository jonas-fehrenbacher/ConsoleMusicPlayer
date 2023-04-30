#pragma once

#include "core/MessageBus.hpp"

enum Message
{
	MenuState_EnteredPlaylist = core::MessageID::CUSTOM_MESSAGE,
	MenuState_SelectedPlaylistEditor,
	PlayState_Finished,
	PlaylistEditorState_Finished
};