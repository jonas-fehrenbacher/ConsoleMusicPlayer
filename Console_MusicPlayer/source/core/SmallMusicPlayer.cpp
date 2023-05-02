#include "core/SmallMusicPlayer.hpp"
#include "core/SmallTools.hpp"
#include "core/InputDevice.hpp"
#include "core/Console.hpp"
#include <Windows.h>
#include <iostream>

core::SmallMusicPlayer::SmallMusicPlayer() :
	playlist(),
	drawSize(),
	drawPosX(),
	drawKeyInfo(true),
	volumeReport(),
	cooldownVolumeReport(),
	skipReport(),
	cooldownSkipReport(),
	replay(Replay::None)
{

}

void core::SmallMusicPlayer::init(std::vector<fs::path> musicDirs, int drawSize, int drawPosX, Style style)
{
	this->drawSize = drawSize;
	this->drawPosX = drawPosX;
	this->style = style;

	std::map<std::wstring, std::wstring> config = core::getConfig("data/config.dat");
	int options = // (config[L"isPlaylistShuffled"] == L"true" ? core::Playlist::Shuffle : 0) |
		          // (config[L"playlistLoop"] == L"true" ? core::Playlist::Loop : 0) |
		          core::Playlist::FadeOut;
	// TODO: PlayOne and combine that with Loop

	playlist.init(musicDirs, options);
}

void core::SmallMusicPlayer::terminate()
{
	playlist.terminate();
}

void core::SmallMusicPlayer::update()
{
	playlist.update();

	//volume report:
	if (cooldownVolumeReport.getElapsedTime().asSeconds() > 0.5f && volumeReport != "")
	{
		volumeReport = core::Text();
		cooldownVolumeReport.restart();
	}

	//skip report:
	if (cooldownSkipReport.getElapsedTime().asSeconds() > 0.5f && skipReport != "")
	{
		skipReport = core::Text();
		cooldownSkipReport.restart();
	}
}

void core::SmallMusicPlayer::handleEvent()
{
	// Up / Down key (or music finished):
	if (!playlist.isCurrentMusicStopped()) {
		if (inputDevice::isKeyPressed(VK_UP)) {
			playlist.playNext();
		}
		else if (inputDevice::isKeyPressed(VK_DOWN)) {
			playlist.playPrevious();
		}
	}

	//R-Key
	if (inputDevice::isKeyPressed('R')) {
		if (playlist.isShuffled()) playlist.resetShuffle();
		else playlist.shuffle();
	}

	//L-Key:
	if (inputDevice::isKeyPressed('L')) { // Loop key
		// ..select one of 3 modi
		replay = static_cast<Replay>(replay + 1);
		replay = static_cast<Replay>(replay % Replay::Count); // wrap around

		if (replay == Replay::None) {
			playlist.setLoop(false);
			playlist.setCurrentMusicLoop(false);
		}
		if (replay == Replay::One) {
			playlist.setLoop(false);
			playlist.setCurrentMusicLoop(true);
		}
		if (replay == Replay::All) {
			playlist.setLoop(true);
			playlist.setCurrentMusicLoop(false);
		}
	}

	//Left-Key:
	if (!playlist.isCurrentMusicStopped() && inputDevice::isKeyPressed(VK_LEFT))
	{
		if (playlist.getCurrentMusicElapsedTime().asSeconds() < 5.f)
		{
			std::string skippedTimeText = std::to_string(playlist.getCurrentMusicElapsedTime().asSeconds());
			skipReport = core::Text(" -" + skippedTimeText.substr(0, skippedTimeText.find(".") + 3) + "sec", core::Color::Light_Red);
			if (playlist.getCurrentMusicLoop()) playlist.skipTime(core::Time(-5s)); // will be 0
			else playlist.playPrevious();
		}
		else
		{
			skipReport = core::Text(" -5sec", core::Color::Light_Red);
			playlist.skipTime(core::Time(-5s));
		}

		cooldownSkipReport.restart();
	}

	//Right-Key:
	if (!playlist.isCurrentMusicStopped() && inputDevice::isKeyPressed(VK_RIGHT))
	{
		if (playlist.getCurrentMusicElapsedTime().asSeconds() > playlist.getCurrentMusicDuration().asSeconds() - 5.f)
		{
			// ..there are no 5sec remaining
			core::Time skippedTime = playlist.getCurrentMusicDuration() - playlist.getCurrentMusicElapsedTime();
			std::string skippedTimeText = std::to_string(skippedTime.asSeconds());
			skipReport = core::Text(" +" + skippedTimeText.substr(0, skippedTimeText.find(".") + 3) + "sec", core::Color::Light_Green);
			if (playlist.getCurrentMusicLoop()) playlist.skipTime(skippedTime); // Set music to the end, so it loops immediately.
			else playlist.playNext();
		}
		else
		{
			skipReport = core::Text(" +5sec", core::Color::Light_Green);
			playlist.skipTime(core::Time(5s));
		}

		cooldownSkipReport.restart();
	}

	//+-Key:
	if (inputDevice::isKeyPressed(VK_OEM_PLUS))
	{
		if (playlist.getVolume() <= 95)
		{
			volumeReport = core::Text("+5%", core::Color::Light_Green);
			playlist.setVolume(playlist.getVolume() + 5);
		}
		else
		{
			volumeReport = core::Text("+" + std::to_string(100 - static_cast<unsigned short>(playlist.getVolume())) + "%", core::Color::Light_Green);
			playlist.setVolume(100);
		}

		cooldownVolumeReport.restart();
	}

	//--Key:
	if (inputDevice::isKeyPressed(VK_OEM_MINUS))
	{
		if (playlist.getVolume() >= 5)
		{
			volumeReport = core::Text("-5%", core::Color::Light_Red);
			playlist.setVolume(playlist.getVolume() - 5);
		}
		else
		{
			volumeReport = core::Text("-" + std::to_string(static_cast<unsigned short>(playlist.getVolume())) + "%", core::Color::Light_Red);
			playlist.setVolume(0);
		}

		cooldownVolumeReport.restart();
	}

	//P-Key:
	if (!playlist.isCurrentMusicStopped() && inputDevice::isKeyPressed('P'))
	{
		if (playlist.isPlaying())
		{
			playlist.pause();
		}
		else if (playlist.isPaused())
		{
			playlist.resume();
		}
	}
}

intern void drawBorder(int size, bool isTop, int posX, core::Color color);
void core::SmallMusicPlayer::draw()
{
	drawBorder(drawSize, true, drawPosX, style.border);
	
	// Music name:
	std::cout << std::string(drawPosX, ' ');
	if (playlist.isCurrentMusicStopped()) {
		std::cout << core::Text("No music selected", style.title);
	}
	else {
		std::string title = playlist.current().title.substr(0, drawSize);
		if (playlist.current().title.length() > drawSize) {
			title.replace(title.end() - 3, title.end(), "...");
		}
		std::cout << core::Text(title, style.title);
	}
	std::cout << core::endl();

	// Duration:
	std::string currDuration = playlist.isCurrentMusicStopped() ? "-" : getTimeStr(playlist.getCurrentMusicElapsedTime(), playlist.getCurrentMusicDuration());
	std::string duration = playlist.isCurrentMusicStopped() ? "-" : getTimeStr(playlist.getCurrentMusicDuration());
	std::string durationInfo = currDuration + skipReport.str + " of " + duration;
	std::string durationSkipForwardKeyInfo = drawKeyInfo ? "["s + core::uc::leftwardsArrow + "] " : "";
	std::string durationSkipBackwardKeyInfo = drawKeyInfo ? " ["s + core::uc::rightwardsArrow + "]" : "";
	int durationKeyInfoLength = durationSkipForwardKeyInfo.length() > 0 ? 8 : 0; // [<] ... [>]; note length is wrong because of unicode characters
	int drawPos = core::console::getCharCount().x / 2.f - (durationInfo.length() + durationKeyInfoLength) / 2.f;
	std::cout << std::string(drawPos, ' ')
		<< core::Text(durationSkipForwardKeyInfo, core::Color::Gray)
		<< core::Text(currDuration, style.duration)
		<< skipReport << core::Text(" of ", style.duration) << core::Text(duration, style.duration)
		<< core::Text(durationSkipBackwardKeyInfo, core::Color::Gray)
		<< core::endl();

	// Play status:
	std::string playStatus = "";
	if (playlist.isPlaying()) {
		playStatus = "Playing";
	}
	else if (playlist.isPaused()) {
		playStatus = "Paused";
	}
	else if (playlist.isCurrentMusicStopped()) {
		playStatus = "Stopped";
	}

	// Loop symbol:
	core::Text loopSymbol = core::Text("L", style.statusOff); // default Replay::None
	if (replay == Replay::One) loopSymbol = core::Text("L1", style.statusOn);
	if (replay == Replay::All) loopSymbol = core::Text("L", style.statusOn);

	// Random symbol:
	core::Text randomSymbol = core::Text("R", playlist.isShuffled() ? style.statusOn : style.statusOff);

	std::string status = "R " + loopSymbol.str + " << < " + playStatus + " > >> " + std::to_string((int)playlist.getVolume()) + "%";
	int statusPos = round((drawPosX + drawSize / 2.f) - status.length() / 2.f);
	status = status.substr(std::string("R " + loopSymbol.str).size());
	std::cout << std::string(statusPos, ' ') << randomSymbol << " " << loopSymbol << core::Text(status, style.status) << " " << volumeReport << core::endl();

	// Key info:
	if (drawKeyInfo)
	{
		float playStatusHalfLength = playStatus.length() / 2.f;
		bool isEvenLength = playStatus.length() / 2.f == playStatus.length() / 2; // on even state the character (P) fits not in the middle
		std::string keyInfo = "[R|L"s + (replay == Replay::One ? " |" : "|") + core::uc::downwardsArrow + " |" + core::uc::leftwardsArrow +
			"|" + std::string(playStatusHalfLength, ' ') + "P" + std::string(playStatusHalfLength - (isEvenLength? 1 : 0), ' ') + "|"
			+ core::uc::rightwardsArrow + "| " + core::uc::upwardsArrow + "|" + (playlist.getVolume() == 100.f ? " " : "") + "+/-]";
		std::cout << std::string(statusPos-1, ' ') << core::Text(keyInfo, core::Color::Gray) << core::endl();
	}

	drawBorder(drawSize, false, drawPosX, style.border);
}

intern void drawBorder(int size, bool isTop, int posX, core::Color color)
{
	std::string border = "";
	for (int i = 0; i < size; ++i) {
		border += isTop ? "_" : core::uc::overline;
	}
	std::cout << std::string(posX, ' ') << core::Text(border, color) << core::endl();
}

void core::SmallMusicPlayer::play(std::string name)
{
	__debugbreak();
	playlist.start();
}

void core::SmallMusicPlayer::play(int index)
{
	playlist.start(index);
}

void core::SmallMusicPlayer::setPosX(int posX)
{
	drawPosX = posX;
}

void core::SmallMusicPlayer::setDrawKeyInfo(bool drawKeyInfo)
{
	this->drawKeyInfo = drawKeyInfo;
}

int core::SmallMusicPlayer::getCurrentMusicIndex()
{
	return playlist.getCurrentMusicIndex();
}

const core::Playlist& core::SmallMusicPlayer::getPlaylist() const
{
	return playlist;
}

bool core::SmallMusicPlayer::isStopped()
{
	return playlist.isCurrentMusicStopped();
}