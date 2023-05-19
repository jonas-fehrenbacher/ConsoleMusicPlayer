#include "PlayStatus.hpp"
#include "App.hpp"
#include "core/Console.hpp"
#include "core/InputDevice.hpp"
#include "core/SmallTools.hpp"
#include <iostream>

void PlayStatus::init(App* app)
{
	this->app = app;
}

intern void coutWithProgressBar(int& progressBarSize, std::string text, core::Color textColor, core::Color progressBarTextColor, core::Color progressBarColor)
{
	std::cout << core::Text(progressBarSize > text.size() ? text : text.substr(0, progressBarSize), progressBarSize > 0 ? progressBarTextColor : textColor, progressBarSize > 0 ? progressBarColor : core::Color::None)
		<< core::Text(progressBarSize > text.size() ? "" : text.substr(progressBarSize, text.size() - progressBarSize), textColor, core::Color::None);
	progressBarSize -= text.size();
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
		core::Text playbackKey = core::Text(app->isDrawKeyInfo ? " [p]" : "", core::Color::Gray);
		// shuffle:
		core::Text shuffleStatus = core::Text("shuffle ", app->musicPlayer.isShuffled() ? style.statusOn : style.statusOff);
		core::Text shuffleKey = core::Text(app->isDrawKeyInfo ? "[r] " : "", core::Color::Gray);
		int shufflePos = core::console::getCharCount().x - (playbackStatus.str.length() + shuffleStatus.str.length() + playbackKey.str.length() + shuffleKey.str.length());
		// volume:
		core::Text volume = core::Text(" vol "s + std::to_string((int)app->musicPlayer.getVolume()) + "%", core::Color::White);
		core::Text volumeKey = core::Text(app->isDrawKeyInfo ? " [+/-]" : "", core::Color::Gray);
		// repeat:
		core::Text repeatStatus = core::Text("repeat off ", style.statusOff);
		core::Text repeatKey = core::Text(app->isDrawKeyInfo ? "[l] " : "", core::Color::Gray);
		if (app->musicPlayer.getReplayStatus() == core::MusicPlayer::Replay::One) repeatStatus = core::Text("repeat track ", style.statusOn);
		else if (app->musicPlayer.getReplayStatus() == core::MusicPlayer::Replay::All) repeatStatus = core::Text("repeat list ", style.statusOn);
		int repeatPos = core::console::getCharCount().x - (volume.str.length() + repeatStatus.str.length() + volumeKey.str.length() + repeatKey.str.length() + app->musicPlayer.volumeReport.str.length());
		
		std::cout << playbackStatus << playbackKey << std::string(shufflePos - 1, ' ') << shuffleKey << shuffleStatus << core::endl()
			<< volume << app->musicPlayer.volumeReport << volumeKey << std::string(repeatPos - 1, ' ') << repeatKey << repeatStatus << core::endl(2);
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
	// BUG: progressBarSize is not accurate - there is still space left after track finishes.
	{
		core::Time currPlaytime = app->musicPlayer.isStopped() ? core::Time() : app->musicPlayer.getPlayingMusicElapsedTime();
		core::Time currPlaytimeMax = app->musicPlayer.isStopped() ? core::Time() : app->musicPlayer.getPlayingMusicInfo().duration;
		std::string currDuration = app->musicPlayer.getActivePlaylistSize() == 0 ? "-" : getTimeStr(currPlaytime, currPlaytimeMax);
		std::string duration = app->musicPlayer.getActivePlaylistSize() == 0 ? "-" : getTimeStr(currPlaytimeMax);
		std::string durationInfo = currDuration + app->musicPlayer.skipReport.str + " / " + duration;
		std::string durationSkipForwardKeyInfo = app->isDrawKeyInfo ? "["s + core::uc::leftwardsArrow + "] " : "";
		std::string durationSkipBackwardKeyInfo = app->isDrawKeyInfo ? " ["s + core::uc::rightwardsArrow + "]" : "";
		int durationKeyInfoLength = durationSkipForwardKeyInfo.length() > 0 ? 8 : 0; // [<] ... [>]; note length is wrong because of unicode characters
		int drawPos = core::console::getCharCount().x / 2.f - (durationInfo.length() + durationKeyInfoLength) / 2.f;
		//musicPlayer.skipReport.bgcolor = core::Color::White; // TODO set it in constructor
		core::console::setBgColor(core::Color::Gray);

		float progressBarFactor = currPlaytime.asSeconds() / (currPlaytimeMax == 0s ? 1 : app->musicPlayer.getPlayingMusicInfo().duration.asSeconds()); // 0..duration
		int progressBarSize = core::console::getCharCount().x * progressBarFactor; // 0..consoleCharCountX

		std::string spaceBefore(drawPos, ' ');
		std::string spaceAfter; // can not be initialized here because of getCursorPos()
		coutWithProgressBar(progressBarSize, spaceBefore, style.durationText, style.durationProgressBarText, style.durationProgressBar);
		coutWithProgressBar(progressBarSize, durationSkipForwardKeyInfo, core::Color::White, style.durationProgressBarText, style.durationProgressBar);
		coutWithProgressBar(progressBarSize, currDuration, style.durationText, style.durationProgressBarText, style.durationProgressBar);
		coutWithProgressBar(progressBarSize, app->musicPlayer.skipReport.str, app->musicPlayer.skipReport.fgcolor, app->musicPlayer.skipReport.bgcolor, style.durationProgressBar);
		coutWithProgressBar(progressBarSize, " / ", style.durationText, style.durationProgressBarText, style.durationProgressBar);
		coutWithProgressBar(progressBarSize, duration, style.durationText, style.durationProgressBarText, style.durationProgressBar);
		coutWithProgressBar(progressBarSize, durationSkipBackwardKeyInfo, core::Color::White, style.durationProgressBarText, style.durationProgressBar);
		spaceAfter = std::string(core::console::getCharCount().x - (core::console::getCursorPos().x + 1), ' ');
		coutWithProgressBar(progressBarSize, spaceAfter, style.durationText, style.durationProgressBarText, style.durationProgressBar);
		std::cout << core::endl();
	}
}