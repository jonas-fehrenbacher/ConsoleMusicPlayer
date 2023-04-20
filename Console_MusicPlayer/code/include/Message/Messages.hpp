#pragma once

#include "Message/MessageBus.hpp"

enum Message
{
	MenuState_EnteredPlaylist,
	MenuState_SelectedPlaylistEditor,
	PlayState_Finished,
	PlaylistEditorState_Finished
};