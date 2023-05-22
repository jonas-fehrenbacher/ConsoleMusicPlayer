#include "Title.hpp"
#include "App.hpp"
#include "core/Console.hpp"
#include "core/InputDevice.hpp"
#include "core/SmallTools.hpp"
#include <iostream>

void Title::init(App* app)
{
	this->app    = app;
	playlistName = "Tracks";
}

void Title::update()
{
	playlistName = app->musicPlayer.getActivePlaylistName();
	if (playlistName == app->musicPlayer.ALL_PLAYLIST_NAME) {
		playlistName = "Tracks";
	}
	else {
		playlistName = playlistName.substr(0, playlistName.length() - 3); // remove ".pl"
	}
}

void Title::draw()
{
	Style style = app->style.title;

	// Set background color:
	core::console::setBgColor(style.background);

	///////////////////////////////////////////////////////////////////////////////
	// Draw first line of the header
	///////////////////////////////////////////////////////////////////////////////
	// Exit info:
	std::string exitSymbol = " <";
	std::string exitSymbolKeyInfo = app->isDrawKeyInfo ? "[" + app->keymap.get(Keymap::Action::Exit).symbol + "]"s : ""; // If you use core::uc::leftwardsArrow, then note that std::string cannot properly handle unicode and its length is wrong.
	// List status:
	std::string trackNum = app->musicPlayer.getActivePlaylistSize() > 0 ? std::to_string(app->musicPlayer.getActivePlaylistCurrentTrackNumber()) : "-";
	//if (musicPlayer.getActivePlaylistCurrentTrackNumber() > musicPlayer.getActivePlaylistSize()) trackNum = std::to_string(musicPlayer.getActivePlaylistSize());
	trackNum += " / " + (app->musicPlayer.getActivePlaylistSize() > 0 ? std::to_string(app->musicPlayer.getActivePlaylistSize()) : "-");
	// skip key info:
	std::string skipForwardKeyInfo = app->isDrawKeyInfo ? "["s + app->keymap.get(Keymap::Action::PrevTrack).symbol + "] " : "";
	std::string skipBackwardKeyInfo = app->isDrawKeyInfo ? " ["s + app->keymap.get(Keymap::Action::NextTrack).symbol + "]" : "";
	// Lock events:
	core::Text lockStatusKeyInfo(app->isDrawKeyInfo ? "[" + app->keymap.get(Keymap::Action::LockInput).symbol + "] " : "", style.keyInfo);
	core::Text lockStatus("", style.lockStatus);
	if (core::inputDevice::isLocked())
		lockStatus.str = "Locked Input";
	else lockStatus.str = "Free Input";
	// Playlist name:
	std::string playlistNameInfo = " " + playlistName;
	{
		float trackInfoHalfLength = (skipBackwardKeyInfo.length() + trackNum.length() + playlistNameInfo.length() + skipForwardKeyInfo.length()) / 2.f;
		int leftSideUsedSize = exitSymbol.length() + exitSymbolKeyInfo.length() + 1; // +1 because of 1 character padding to the left side.
		int rightSizeUsedSize = lockStatusKeyInfo.str.length() + lockStatus.str.length() + 1; // +1 because of 1 character padding to the right side.
		int smallHalfSize = leftSideUsedSize > rightSizeUsedSize ? leftSideUsedSize : rightSizeUsedSize;
		float availableHalfLength = core::console::getCharCount().x / 2.f - smallHalfSize;
		if (trackInfoHalfLength > availableHalfLength) {
			int availablePlaylistNameHalfSize = round(availableHalfLength - (skipBackwardKeyInfo.length() + trackNum.length() + skipForwardKeyInfo.length()) / 2.f);
			int offset = 6; // let some space between - do not use full possible length of playlistName
			playlistNameInfo = playlistNameInfo.substr(0, (availablePlaylistNameHalfSize * 2) - offset - 2) + "..";
		}
	}
	// Set draw position:
	int trackNumKeyInfoLength = skipForwardKeyInfo.length() > 0 ? 8 : 0; // [^] ... [v]; note length is wrong because of unicode characters
	int trackNumberDrawPos = core::console::getCharCount().x / 2.f - (trackNum.length() + trackNumKeyInfoLength + playlistNameInfo.length()) / 2.f;
	trackNumberDrawPos -= (exitSymbol.length() + exitSymbolKeyInfo.length());
	// Draw:
	std::cout << core::Text(exitSymbol, style.exitSymbol) << core::Text(exitSymbolKeyInfo, style.keyInfo);
	if (!app->musicPlayer.isStopped()) {
		// ..draw track number if music is playing.
		std::cout << core::Text(std::string(trackNumberDrawPos, ' '))
			<< core::Text(skipForwardKeyInfo, style.keyInfo)
			<< core::Text(trackNum, style.trackNumber) << core::Text(playlistNameInfo, style.playlistName)
			<< core::Text(skipBackwardKeyInfo, style.keyInfo);
	}
	// Draw lock events:
	int drawPos = (core::console::getCharCount().x - core::console::getCursorPos().x) - (lockStatusKeyInfo.str.length() + lockStatus.str.length());
	std::cout << std::string(drawPos - 1, ' ') << lockStatusKeyInfo << lockStatus;
	std::cout << core::endl(core::endl::Mod::ForceLastCharDraw);

	///////////////////////////////////////////////////////////////////////////////
	// Draw track or title of the header
	///////////////////////////////////////////////////////////////////////////////
	if (!app->musicPlayer.isStopped()) {
		// Track name:
		const int maxNameSize = core::console::getCharCount().x - 4; // -4 because I want some padding
		std::string trackName = app->musicPlayer.getPlayingMusicInfo().title.substr(0, maxNameSize);
		if (trackName.length() < app->musicPlayer.getPlayingMusicInfo().title.length()) {
			trackName.replace(trackName.end() - 2, trackName.end(), "..");
		}
		drawPos = core::console::getCharCount().x / 2.f - trackName.length() / 2.f;
		trackName.insert(0, drawPos, ' ');
		std::cout << core::endl() << core::Text(trackName, style.trackName) << core::endl(2);
	}
	else {
		// Title:
		std::string title = "Console Music Player "s + core::uc::eighthNote;
		drawPos = core::console::getCharCount().x / 2.f - (title.length() - 2) / 2.f; // -2 because std::string cannot handle unicode characters - the length is to much.
		title.insert(0, drawPos, ' ');
		// Notice:
		std::string notice = "(Select and press " + app->keymap.get(Keymap::Action::Select).symbol + ")";
		drawPos = core::console::getCharCount().x / 2.f - notice.length() / 2.f;
		notice.insert(0, drawPos, ' ');
		// Draw:
		std::cout << core::Text(title, style.title) << core::endl()
			<< core::Text(app->isDrawKeyInfo ? notice : "", style.keyInfo) << core::endl()
			<< core::endl();
	}
}