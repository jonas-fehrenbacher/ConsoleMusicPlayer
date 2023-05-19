#include "Title.hpp"
#include "App.hpp"
#include "core/Console.hpp"
#include "core/InputDevice.hpp"
#include "core/SmallTools.hpp"
#include <iostream>

void Title::init(App* app)
{
	this->app = app;
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
	std::string exitSymbolKeyInfo = app->isDrawKeyInfo ? "[ESC]"s : ""; // If you use core::uc::leftwardsArrow, then note that std::string cannot properly handle unicode and its length is wrong.
	// List status:
	std::string trackNum = app->musicPlayer.getActivePlaylistSize() > 0 ? std::to_string(app->musicPlayer.getActivePlaylistCurrentTrackNumber()) : "-";
	//if (musicPlayer.getActivePlaylistCurrentTrackNumber() > musicPlayer.getActivePlaylistSize()) trackNum = std::to_string(musicPlayer.getActivePlaylistSize());
	trackNum += " / " + (app->musicPlayer.getActivePlaylistSize() > 0 ? std::to_string(app->musicPlayer.getActivePlaylistSize()) : "-");
	// skip key info:
	std::string skipForwardKeyInfo = app->isDrawKeyInfo ? "["s + core::uc::downwardsArrow + "] " : "";
	std::string skipBackwardKeyInfo = app->isDrawKeyInfo ? " ["s + core::uc::upwardsArrow + "]" : "";
	// Set draw position:
	int trackNumKeyInfoLength = skipForwardKeyInfo.length() > 0 ? 8 : 0; // [^] ... [v]; note length is wrong because of unicode characters
	int trackNumberDrawPos = core::console::getCharCount().x / 2.f - (trackNum.length() + trackNumKeyInfoLength) / 2.f;
	trackNumberDrawPos -= (exitSymbol.length() + exitSymbolKeyInfo.length());
	// Draw:
	std::cout << core::Text(exitSymbol, style.exitSymbol) << core::Text(exitSymbolKeyInfo, style.keyInfo);
	if (!app->musicPlayer.isStopped()) {
		// ..draw track number if music is playing.
		std::cout << core::Text(std::string(trackNumberDrawPos, ' '))
			<< core::Text(skipForwardKeyInfo, style.keyInfo)
			<< core::Text(trackNum, style.trackNumber)
			<< core::Text(skipBackwardKeyInfo, style.keyInfo);
	}
	// Lock events:
	core::Text lockStatusKeyInfo(app->isDrawKeyInfo ? "[F12] " : "", style.keyInfo);
	core::Text lockStatus("", style.lockStatus);
	if (core::inputDevice::isLocked())
		lockStatus.str = "Locked Input";
	else lockStatus.str = "Free Input";
	int drawPos = (core::console::getCharCount().x - core::console::getCursorPos().x) - (lockStatusKeyInfo.str.length() + lockStatus.str.length());
	std::cout << core::Text(std::string(drawPos - 1, ' '))
		<< lockStatusKeyInfo << lockStatus << core::endl();

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
		std::cout << core::endl() << trackName << core::endl(2);
	}
	else {
		// Title:
		std::string title = "Console Music Player "s + core::uc::eighthNote;
		drawPos = core::console::getCharCount().x / 2.f - (title.length() - 2) / 2.f; // -2 because std::string cannot handle unicode characters - the length is to much.
		title.insert(0, drawPos, ' ');
		// Notice:
		std::string notice = "(Select and press enter)";
		drawPos = core::console::getCharCount().x / 2.f - notice.length() / 2.f;
		notice.insert(0, drawPos, ' ');
		// Draw:
		std::cout << core::Text(title, style.title) << core::endl()
			<< core::Text(app->isDrawKeyInfo ? notice : "", style.keyInfo) << core::endl()
			<< core::endl();
	}
}