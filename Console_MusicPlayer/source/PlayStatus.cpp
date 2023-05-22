#include "PlayStatus.hpp"
#include "App.hpp"
#include "core/Console.hpp"
#include "core/InputDevice.hpp"
#include "core/SmallTools.hpp"
#include <iostream>
#include <cassert>

void PlayStatus::init(App* app)
{
	this->app = app;
}

intern void coutWithProgressBar(int& progressBarSize, std::string text, core::Color textColor, core::Color progressBarTextColor, core::Color progressBarColor)
{
	core::Vec2 cursorStartPos = core::console::getCursorPos();

	std::cout << core::Text(progressBarSize > text.size() ? text : text.substr(0, progressBarSize), progressBarSize > 0 ? progressBarTextColor : textColor, progressBarSize > 0 ? progressBarColor : core::Color::None)
		<< core::Text(progressBarSize > text.size() ? "" : text.substr(progressBarSize, text.size() - progressBarSize), textColor, core::Color::None);

	// Note I can not just do 'progressBarSize -= text.size();', because this does not work with unicode characters. The size of those can not be read - not even with std::wstring!
	// So I calculate the size by substracting the cursor positions.
	assert(cursorStartPos.y == core::console::getCursorPos().y);
	progressBarSize -= (core::console::getCursorPos().x - cursorStartPos.x);
}

void PlayStatus::drawDurationBar(int size, std::string label, core::Time elapsedTime, core::Time duration, bool isDrawKeyInfo)
{
	PlayStatus::Style style = app->style.playStatus;

	label.insert(0, " "); // for padding
	int consoleCursorStartPos = core::console::getCursorPos().x;
	std::string currDuration = app->musicPlayer.getActivePlaylistSize() == 0 ? "-" : getTimeStr(elapsedTime, duration);
	std::string durationStr = app->musicPlayer.getActivePlaylistSize() == 0 ? "-" : getTimeStr(duration);
	std::string durationInfo = currDuration + app->musicPlayer.getSkipReport().text + " / " + durationStr;
	std::string durationSkipForwardKeyInfo = isDrawKeyInfo ? "["s + app->keymap.get(Keymap::Action::TrackSkipBackward).symbol + "] " : "";
	std::string durationSkipBackwardKeyInfo = isDrawKeyInfo ? " ["s + app->keymap.get(Keymap::Action::TrackSkipForward).symbol + "]" : "";
	int durationKeyInfoLength = durationSkipForwardKeyInfo.length() > 0 ? 8 : 0; // [<] ... [>]; note length is wrong because of unicode characters
	int drawPos = size / 2.f - (durationInfo.length() + durationKeyInfoLength) / 2.f;
	drawPos -= label.length();

	float progressBarFactor = elapsedTime.asSeconds() / (duration == 0s ? 1 : duration.asSeconds()); // 0..duration
	int progressBarSize = round((float)size * progressBarFactor); // 0..consoleCharCountX

	std::string spaceBefore(drawPos, ' ');
	std::string spaceAfter; // can not be initialized here because of getCursorPos()
	core::Color skipReportColor = app->musicPlayer.getSkipReport().isPositive ? style.skipForwardReport : style.skipBackwardReport;
	core::console::setBgColor(style.background);
	coutWithProgressBar(progressBarSize, label,                                 style.label,                         style.progressBar_label,             style.durationProgressBar);
	coutWithProgressBar(progressBarSize, spaceBefore,                           style.durationText,                  style.progressBar_durationText,      style.durationProgressBar);
	coutWithProgressBar(progressBarSize, durationSkipForwardKeyInfo,            core::Color::White,                  style.progressBar_durationText,      style.durationProgressBar);
	coutWithProgressBar(progressBarSize, currDuration,                          style.durationText,                  style.progressBar_durationText,      style.durationProgressBar);
	coutWithProgressBar(progressBarSize, app->musicPlayer.getSkipReport().text, skipReportColor,                     skipReportColor,                     style.durationProgressBar);
	coutWithProgressBar(progressBarSize, " / ",                                 style.durationText,                  style.progressBar_durationText,      style.durationProgressBar);
	coutWithProgressBar(progressBarSize, durationStr,                           style.durationText,                  style.progressBar_durationText,      style.durationProgressBar);
	coutWithProgressBar(progressBarSize, durationSkipBackwardKeyInfo,           core::Color::White,                  style.progressBar_durationText,      style.durationProgressBar);
	spaceAfter = std::string(size - (core::console::getCursorPos().x - consoleCursorStartPos), ' ');
	coutWithProgressBar(progressBarSize, spaceAfter,                            style.durationText,                  style.progressBar_durationText,      style.durationProgressBar);
}

void PlayStatus::draw()
{
	PlayStatus::Style style = app->style.playStatus;

	///////////////////////////////////////////////////////////////////////////////
	// Track status
	///////////////////////////////////////////////////////////////////////////////
	{
		int lineCount = core::console::getCharCount().y - 4;
		for (int i = core::console::getCursorPos().y; i < lineCount - 1; ++i) {
			std::cout << core::endl();
		}

		// playback:
		core::Text playbackStatus = core::Text(" playback is stopped", style.statusOff);
		if (app->musicPlayer.isPlaying()) playbackStatus = core::Text(" playing", style.statusOn);
		else if (app->musicPlayer.isPaused()) playbackStatus = core::Text(" paused", core::Color::Aqua);
		core::Text playbackKey = core::Text(app->isDrawKeyInfo ? " [" + app->keymap.get(Keymap::Action::PlayPause).symbol + "]" : "", core::Color::Gray);
		// shuffle:
		core::Text shuffleStatus = core::Text("shuffle ", app->musicPlayer.isShuffled() ? style.statusOn : style.statusOff);
		core::Text shuffleKey = core::Text(app->isDrawKeyInfo ? "[" + app->keymap.get(Keymap::Action::Shuffle).symbol + "] " : "", core::Color::Gray);
		int shufflePos = core::console::getCharCount().x - (playbackStatus.str.length() + shuffleStatus.str.length() + playbackKey.str.length() + shuffleKey.str.length());
		// volume:
		core::Text volume = core::Text(" vol "s + std::to_string((int)app->musicPlayer.getVolume()) + "%", core::Color::White);
		core::Text volumeKey = core::Text(app->isDrawKeyInfo ? " [" + app->keymap.get(Keymap::Action::IncreaseVolume).symbol + "/" + app->keymap.get(Keymap::Action::DecreaseVolume).symbol + "]" : "", core::Color::Gray);
		// repeat:
		core::Text repeatStatus = core::Text("repeat off ", style.statusOff);
		core::Text repeatKey = core::Text(app->isDrawKeyInfo ? "[" + app->keymap.get(Keymap::Action::Repeat).symbol + "] " : "", core::Color::Gray);
		if (app->musicPlayer.getReplayStatus() == core::MusicPlayer::Replay::One) repeatStatus = core::Text("repeat track ", style.statusOn);
		else if (app->musicPlayer.getReplayStatus() == core::MusicPlayer::Replay::All) repeatStatus = core::Text("repeat list ", style.statusOn);
		int repeatPos = core::console::getCharCount().x - (volume.str.length() + repeatStatus.str.length() + volumeKey.str.length() + repeatKey.str.length() + app->musicPlayer.getVolumeReport().text.length());
		
		std::cout << playbackStatus << playbackKey << std::string(shufflePos - 1, ' ') << shuffleKey << shuffleStatus << core::endl(core::endl::Mod::ForceLastCharDraw)
			<< volume << core::Text(app->musicPlayer.getVolumeReport().text, app->musicPlayer.getVolumeReport().isPositive ? style.volumePlusReport : style.volumeMinusReport)
			<< volumeKey << std::string(repeatPos - 1, ' ') << repeatKey << repeatStatus << core::endl(2, core::Color::None, core::endl::Mod::ForceLastCharDraw);
	}
	/*
		if (playlist.getCurrentMusicLoop())
			std::cout << " [loop]";
		else
		{
			if (playlist.current().playCount == 1)
				std::cout << " [" << core::Text("1", core::Color::Bright_White) << " time]";
			else std::cout << " [" << core::Text(std::to_string(playlist.current().playCount), core::Color::Bright_White) << " times]";
		}
		std::cout << core::Text(app->isDrawKeyInfo ? " [W/S; (L)oop]" : "", core::Color::Gray) << core::endl();
	*/

	///////////////////////////////////////////////////////////////////////////////
	// Duration
	///////////////////////////////////////////////////////////////////////////////
	std::string trackLabel = "Track: ";
	std::string playlistLabel = "Playlist: ";
	float durationBarSize = (core::console::getCharCount().x / 2.f);
	// Track duration bar:
	core::Time currPlaytime = app->musicPlayer.isStopped() ? core::Time() : app->musicPlayer.getPlayingMusicElapsedTime();
	core::Time currPlaytimeMax = app->musicPlayer.isStopped() ? core::Time() : app->musicPlayer.getPlayingMusicInfo().duration;
	drawDurationBar(floor(durationBarSize), "Track", currPlaytime, currPlaytimeMax, app->isDrawKeyInfo);
	std::cout << core::Text("|", core::Color::Bright_White, core::Color::White);
	// Playlist duration bar:
	core::Time currPlaylistPlaytime = app->musicPlayer.isStopped() ? core::Time() : app->musicPlayer.getActivePlaylistPlaytime();
	core::Time currPlaylistPlaytimeMax = app->musicPlayer.isStopped() ? core::Time() : app->musicPlayer.getActivePlaylistDuration();
	drawDurationBar(round(durationBarSize) - 1, "Playlist", currPlaylistPlaytime, currPlaylistPlaytimeMax, false);
	std::cout << core::endl();
}