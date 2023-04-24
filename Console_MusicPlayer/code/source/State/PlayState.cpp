#include "State/PlayState.hpp"
#include <conio.h>
#include "Tools/InputDevice.hpp"
#include "Tools/Tool.hpp"
#include "Message/Messages.hpp"
#include "Console.hpp"
#include "State/MenuState.hpp"
#include "App.hpp"
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

PlayState::PlayState(App* app) :
	app(app),
	volumeReport("", core::Color::White),
	cooldownVolumeReport(),
	skipReport("", core::Color::Gray),
	cooldownSkipReport(),
	totalRunTime(0s),
	totalRunTimeClock(),
	playlist(),
	isPrintOrder(false),
	isPrintKeyInfo(false)
{

}

void PlayState::init()
{
	std::map<std::wstring, std::wstring> config = core::getConfig("data/config.dat");

	int options = (config[L"isPlaylistShuffled"] == L"true" ? core::Playlist::Shuffle : 0) | 
		          (config[L"playlistLoop"]       == L"true" ? core::Playlist::Loop : 0)    | 
		          core::Playlist::FadeOut;

	playlist.init(app->menuState.getPlaylistPath(), app->musicDirs, options);

	if (config[L"totalRuntime"] != L"nolimit") {
		totalRunTime = core::Seconds(std::stoi(config[L"totalRuntime"]));
	}

	totalRunTimeClock.restart();
}

void PlayState::terminate()
{
	volumeReport = core::ColoredStr("", core::Color::White);
	skipReport = core::ColoredStr("", core::Color::Gray);
	totalRunTime = 0s;
	playlist.terminate();
	isPrintOrder = false;
	isPrintKeyInfo = false;
}

void PlayState::update()
{
	if (playlist.isStopped() && playlist.size() > 0) {
		// If playlist.size() == 0, then notify user instead of returning to the menu.
		app->messageBus.send(Message::PlayState_Finished);
	}

	//total runtime:
	if (totalRunTime > 0s && totalRunTimeClock.getElapsedTime().asSeconds() >= totalRunTime.asSeconds())
	{
		app->messageBus.send(Message::PlayState_Finished);
		playlist.stop();
	}

	playlist.update();
}

void PlayState::handleEvent()
{
	using namespace core;

	// Up / Down key (or music finished):
	if (inputDevice::isKeyPressed(VK_UP) || playlist.isCurrentMusicStopped()) {
		playlist.playNext();
	}
	else if (inputDevice::isKeyPressed(VK_DOWN)) {
		playlist.playPrevious();
	}

	//W-Key:
	if (inputDevice::isKeyPressed('W'))
		++playlist.current().playCount;

	//S-Key:
	if (inputDevice::isKeyPressed('S'))
		if (playlist.current().playCount > 1)
			--playlist.current().playCount;

	//F12-Key:
	if (inputDevice::isKeyPressed(VK_F12, true))
		inputDevice::lock(!inputDevice::isLocked());

	//I-Key:
	if (inputDevice::isKeyPressed('I'))
		isPrintKeyInfo = !isPrintKeyInfo;

	//E-Key:
	if (inputDevice::isKeyPressed('E'))
		playlist.setLoop(!playlist.getLoop());

	//L-Key
	if (inputDevice::isKeyPressed('L'))
		playlist.currentMusicSetLoop(!playlist.currentMusicGetLoop());

	//R-Key
	if (inputDevice::isKeyPressed('R'))
		playlist.shuffle();

	//O-Key:
	if (inputDevice::isKeyPressed('O'))
		isPrintOrder = !isPrintOrder;

	//B-Key:
	if (inputDevice::isKeyPressed('B'))
		app->messageBus.send(Message::PlayState_Finished);

	//ESC:
	if (inputDevice::isKeyPressed(VK_ESCAPE, false))
	{
		// TODO Send message: Playlist stopped
		playlist.stop();
	}

	//Right-Key:
	if (inputDevice::isKeyPressed(VK_RIGHT))
	{
		if (playlist.currentMusicElapsedTime().asSeconds() > playlist.currentMusicDuration().asSeconds() - 5.f)
		{
			// ..there are no 5sec remaining
			core::Time skippedTime = playlist.currentMusicDuration() - playlist.currentMusicElapsedTime();
			std::string skippedTimeText = std::to_string(skippedTime.asSeconds());
			skipReport = core::ColoredStr(" +" + skippedTimeText.substr(0, skippedTimeText.find(".") + 3) + "sec", core::Color::Light_Green);
			if (playlist.currentMusicGetLoop()) playlist.skipTime(skippedTime); // Set music to the end, so it loops immediately.
			else playlist.playNext();
		}
		else
		{
			skipReport = core::ColoredStr(" +5sec", core::Color::Light_Green);
			playlist.skipTime(core::Time(5s));
		}

		cooldownSkipReport.restart();
	}

	//Left-Key:
	if (inputDevice::isKeyPressed(VK_LEFT))
	{
		if (playlist.currentMusicElapsedTime().asSeconds() < 5.f)
		{
			std::string skippedTimeText = std::to_string(playlist.currentMusicElapsedTime().asSeconds());
			skipReport = core::ColoredStr(" -" + skippedTimeText.substr(0, skippedTimeText.find(".") + 3) + "sec", core::Color::Light_Red);
			if (playlist.currentMusicGetLoop()) playlist.skipTime(core::Time(-5s)); // will be 0
			else playlist.playPrevious();
		}
		else
		{
			skipReport = core::ColoredStr(" -5sec", core::Color::Light_Red);
			playlist.skipTime(core::Time(-5s));
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

	//P-Key:
	if (inputDevice::isKeyPressed('P'))
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

std::string getTimeStr(core::Time time, core::Time limit = 0ns)
{
	if (limit == 0s) {
		limit = time;
	}

	bool hasHours = false;
	bool hasMinutes = false;
	std::string timeStr = "";
	if (static_cast<int>(limit.asHour()) > 0) {
		int hours = (int)time.asHour();
		timeStr += std::to_string(hours) + ":";
		time -= core::Hours((int)time.asHour());
		hasHours = true;
	}
	if (static_cast<int>(limit.asMinute()) > 0) {
		int minutes = (int)time.asMinute();
		timeStr += (hasHours && minutes < 10 ? "0" : "") + std::to_string(minutes) + ":";
		time -= core::Minutes((int)time.asMinute());
		hasMinutes = true;
	}
	if (static_cast<int>(limit.asSeconds()) > 0) { // note: don't do time > 0s, because an 1min song has at this point time==0
		int seconds = (int)time.asSeconds();
		timeStr += (hasMinutes && seconds < 10 ? "0" : "") + std::to_string(seconds);
		time -= core::Seconds((int)time.asSeconds());
	}
	return timeStr;
}

void PlayState::draw()
{
	if (playlist.size() == 0) {
		std::cout << core::ColoredStr("Oooops! There is no music!", core::Color::Bright_White) << core::endl();
		std::cout << "Playlist is empty or music could not be found!" << core::endl() << "Searched for music in:" << core::endl();
		for (auto& musicDir : app->musicDirs) {
			std::wcout << L"- " << musicDir.wstring() << core::endl();
		}
		std::cout << core::endl();
	}
	else {
		//num:
		if (playlist.currentNumber() <= playlist.size())
			std::cout << playlist.currentNumber();
		else std::cout << playlist.size();

		//title
		std::cout << "/" << playlist.size() << ". " << core::ColoredStr(playlist.current().title, core::Color::Light_Yellow) << ":"
			<< core::endl() << core::endl();

		//time:
		std::cout << "Time:           ";
		std::cout << getTimeStr(playlist.currentMusicElapsedTime(), playlist.currentMusicDuration());
		std::cout << skipReport << " of " << getTimeStr(playlist.currentMusicDuration()) << core::endl();
		//if (playlist.currentMusicElapsedTime().asSeconds() < 60) std::cout << static_cast<unsigned short>(playlist.currentMusicElapsedTime().asSeconds()) << core::ColoredStr("sec", core::Color::Gray);
		//if (playlist.currentMusicElapsedTime().asSeconds() >= 60) std::cout << std::setprecision(3) << playlist.currentMusicElapsedTime().asSeconds() / 60.f << core::ColoredStr("min", core::Color::Gray);
		//std::cout << skipReport << " of " << std::setprecision(3) << playlist.currentMusicDuration().asSeconds() / 60.f << core::ColoredStr("min", core::Color::Gray) << core::endl();

		//artist:
		std::cout << "Artist:         " << playlist.current().artist << core::endl();

		//album:
		std::cout << "Album:          " << playlist.current().album << core::endl();

		//volume:			          
		std::cout << "Volume:         " << playlist.getVolume() << "% " << volumeReport << core::endl();

		//state:
		if (playlist.isPaused())
			std::cout << "State:          Paused";
		else if (playlist.isPlaying())
			std::cout << "State:          Playing";
		else if (playlist.isStopped())
			std::cout << "State:          Stopped";
		if (playlist.currentMusicGetLoop()) std::cout << " [loop]" << core::endl();
		else
		{
			if (playlist.current().playCount == 1)
				std::cout << " [" << core::ColoredStr("1", core::Color::Bright_White) << " time]" << core::endl();
			else std::cout << " [" << core::ColoredStr(std::to_string(playlist.current().playCount), core::Color::Bright_White) << " times]" << core::endl();
		}

		//playlist state:
		if (playlist.getLoop()) std::cout << "Playlist State: endless" << core::endl();
		else std::cout << "Playlist State: not endless" << core::endl();

		//total time:
		if (totalRunTime.asSeconds() == 0.f)
			std::cout << "Total runtime:  no limit" << core::endl();
		else
		{
			std::cout << "Total runtime:  ";
			if (totalRunTimeClock.getElapsedTime().asSeconds() < 60.f)
				std::cout << static_cast<unsigned short>(totalRunTimeClock.getElapsedTime().asSeconds()) << core::ColoredStr("sec", core::Color::Gray) << " von ";
			else std::cout << totalRunTimeClock.getElapsedTime().asMinute() << core::ColoredStr("min", core::Color::Gray) << " von ";
			if (totalRunTime.asSeconds() < 60.f)
				std::cout << static_cast<unsigned short>(totalRunTime.asSeconds()) << core::ColoredStr("sec", core::Color::Gray);
			else std::cout << totalRunTime.asSeconds() / 60.f << core::ColoredStr("min", core::Color::Gray);
			std::cout << core::endl();
		}
	}

	//lock events:
	if (core::inputDevice::isLocked())
		std::cout << "Events:         " << core::ColoredStr("Locked", core::Color::Light_Red) << core::endl();
	else std::cout << "Events:         " << core::ColoredStr("Free", core::Color::Light_Green) << core::endl();

	//key info:
	if (isPrintKeyInfo)
	{
		std::cout << core::endl() << core::ColoredStr("Key Info:", core::Color::Light_Aqua) << core::endl();
		std::cout << core::ColoredStr("+     => +5% volume", core::Color::Bright_White) << core::endl()
			<< core::ColoredStr("-     => -5% volume", core::Color::White) << core::endl()
			<< core::ColoredStr("Right => +5sec", core::Color::Bright_White) << core::endl()
			<< core::ColoredStr("Left  => -5sec", core::Color::White) << core::endl()
			<< core::ColoredStr("Up    => next song", core::Color::Bright_White) << core::endl()
			<< core::ColoredStr("Down  => previous song", core::Color::White) << core::endl()
			<< core::ColoredStr("P     => play/pause", core::Color::Bright_White) << core::endl()
			<< core::ColoredStr("O     => Song order", core::Color::White) << core::endl()
			<< core::ColoredStr("I     => Key Info", core::Color::Bright_White) << core::endl()
			<< core::ColoredStr("E     => Playlist Endless", core::Color::White) << core::endl()
			<< core::ColoredStr("R     => Randomize playlist", core::Color::White) << core::endl()
			<< core::ColoredStr("L     => Song Loop", core::Color::Bright_White) << core::endl()
			<< core::ColoredStr("W     => +1 time on current song", core::Color::White) << core::endl()
			<< core::ColoredStr("S     => -1 time on current song (min. 1)", core::Color::Bright_White) << core::endl()
			<< core::ColoredStr("B     => Back to menu", core::Color::White) << core::endl()
			<< core::ColoredStr("ESC   => Exit", core::Color::White) << core::endl()
			<< core::ColoredStr("F12   => Lock Events", core::Color::Bright_White) << core::endl();
	}
	else std::cout << core::endl() << core::ColoredStr("[Press [I]nfo]", core::Color::Light_Aqua) << core::endl();

	//song order:
	if (isPrintOrder)
	{
		std::cout << core::endl() << core::endl() << core::endl();

		bool drawing = true;
		for (size_t i = 0; i < playlist.size(); ++i, drawing = !drawing) {
			std::cout << (i + 1) << ". " << core::ColoredStr(playlist.at(i).title, drawing ? core::Color::Bright_White : core::Color::White) << core::endl();
		}

		std::cout << core::ColoredStr("Console Freeze", core::Color::Light_Aqua);
		_getch();
	}
}