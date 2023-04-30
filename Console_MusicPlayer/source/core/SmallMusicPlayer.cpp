#include "core/SmallMusicPlayer.hpp"
#include "core/SmallTools.hpp"
#include "core/InputDevice.hpp"
#include "core/Console.hpp"
#include <Windows.h>

core::SmallMusicPlayer::SmallMusicPlayer() :
	playlist(),
	drawSize(),
	drawPosX(),
	drawKeyInfo(true),
	volumeReport("", core::Color::White),
	cooldownVolumeReport(),
	skipReport("", core::Color::Gray),
	cooldownSkipReport(),
	replay(Replay::None)
{

}

void core::SmallMusicPlayer::init(std::vector<fs::path> musicDirs, int drawSize, int drawPosX)
{
	this->drawSize = drawSize;
	this->drawPosX = drawPosX;

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
		volumeReport = core::ColoredStr("", core::Color::Gray);
		cooldownVolumeReport.restart();
	}

	//skip report:
	if (cooldownSkipReport.getElapsedTime().asSeconds() > 0.5f && skipReport != "")
	{
		skipReport = core::ColoredStr("", core::Color::Gray);
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
			skipReport = core::ColoredStr(" -" + skippedTimeText.substr(0, skippedTimeText.find(".") + 3) + "sec", core::Color::Light_Red);
			if (playlist.getCurrentMusicLoop()) playlist.skipTime(core::Time(-5s)); // will be 0
			else playlist.playPrevious();
		}
		else
		{
			skipReport = core::ColoredStr(" -5sec", core::Color::Light_Red);
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
			skipReport = core::ColoredStr(" +" + skippedTimeText.substr(0, skippedTimeText.find(".") + 3) + "sec", core::Color::Light_Green);
			if (playlist.getCurrentMusicLoop()) playlist.skipTime(skippedTime); // Set music to the end, so it loops immediately.
			else playlist.playNext();
		}
		else
		{
			skipReport = core::ColoredStr(" +5sec", core::Color::Light_Green);
			playlist.skipTime(core::Time(5s));
		}

		cooldownSkipReport.restart();
	}

	//+-Key:
	if (inputDevice::isKeyPressed(VK_OEM_PLUS))
	{
		if (playlist.getVolume() <= 95)
		{
			volumeReport = core::ColoredStr("+5%", core::Color::Light_Green);
			playlist.setVolume(playlist.getVolume() + 5);
		}
		else
		{
			volumeReport = core::ColoredStr("+" + std::to_string(100 - static_cast<unsigned short>(playlist.getVolume())) + "%", core::Color::Light_Green);
			playlist.setVolume(100);
		}

		cooldownVolumeReport.restart();
	}

	//--Key:
	if (inputDevice::isKeyPressed(VK_OEM_MINUS))
	{
		if (playlist.getVolume() >= 5)
		{
			volumeReport = core::ColoredStr("-5%", core::Color::Light_Red);
			playlist.setVolume(playlist.getVolume() - 5);
		}
		else
		{
			volumeReport = core::ColoredStr("-" + std::to_string(static_cast<unsigned short>(playlist.getVolume())) + "%", core::Color::Light_Red);
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

intern void drawBorder(int size, bool isTop, int posX);
void core::SmallMusicPlayer::draw()
{
	drawBorder(drawSize, true, drawPosX);
	
	// Music name:
	std::cout << std::string(drawPosX, ' ');
	if (playlist.isCurrentMusicStopped()) {
		std::cout << core::ColoredStr("No music selected", core::Color::Gray);
	}
	else {
		std::string title = playlist.current().title.substr(0, drawSize);
		if (playlist.current().title.length() > drawSize) {
			title.replace(title.end() - 3, title.end(), "...");
		}
		std::cout << core::ColoredStr(title, core::Color::Gray);
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
		<< core::ColoredStr(durationSkipForwardKeyInfo, core::Color::Gray)
		<< core::ColoredStr(currDuration, core::Color::White)
		<< skipReport << core::ColoredStr(" of ", core::Color::White) << core::ColoredStr(duration, core::Color::White)
		<< core::ColoredStr(durationSkipBackwardKeyInfo, core::Color::Gray)
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
	core::ColoredStr loopSymbol = core::ColoredStr("L", core::Color::Light_Red); // default Replay::None
	if (replay == Replay::One) loopSymbol = core::ColoredStr("L1", core::Color::Green);
	if (replay == Replay::All) loopSymbol = core::ColoredStr("L", core::Color::Green);

	// Random symbol:
	core::ColoredStr randomSymbol = core::ColoredStr("R", playlist.isShuffled() ? core::Color::Green : core::Color::Light_Red);

	std::string status = "R " + loopSymbol.str + " << < " + playStatus + " > >> " + std::to_string((int)playlist.getVolume()) + "%";
	int statusPos = round((drawPosX + drawSize / 2.f) - status.length() / 2.f);
	status = status.substr(std::string("R " + loopSymbol.str).size());
	std::cout << std::string(statusPos, ' ') << randomSymbol << " " << loopSymbol << status << " " << volumeReport << core::endl();

	// Key info:
	if (drawKeyInfo)
	{
		float playStatusHalfLength = playStatus.length() / 2.f;
		bool isEvenLength = playStatus.length() / 2.f == playStatus.length() / 2; // on even state the character (P) fits not in the middle
		std::string keyInfo = "[R|L"s + (replay == Replay::One ? " |" : "|") + core::uc::downwardsArrow + " |" + core::uc::leftwardsArrow +
			"|" + std::string(playStatusHalfLength, ' ') + "P" + std::string(playStatusHalfLength - (isEvenLength? 1 : 0), ' ') + "|"
			+ core::uc::rightwardsArrow + "| " + core::uc::upwardsArrow + "|" + (playlist.getVolume() == 100.f ? " " : "") + "+/-]";
		std::cout << std::string(statusPos-1, ' ') << core::ColoredStr(keyInfo, core::Color::Gray) << core::endl();
	}

	drawBorder(drawSize, false, drawPosX);
}

intern void drawBorder(int size, bool isTop, int posX)
{
	std::string border = "";
	for (int i = 0; i < size; ++i) {
		border += isTop ? "_" : core::uc::overline;
	}
	std::cout << std::string(posX, ' ') << core::ColoredStr(border, core::Color::Gray) << core::endl();
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