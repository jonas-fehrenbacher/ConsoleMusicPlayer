#include "State/PlayState.hpp"
#include "Messages.hpp"
#include "App.hpp"
#include "core/InputDevice.hpp"
#include "core/SmallTools.hpp"
#include "core/Console.hpp"
#include "core/Playlist.hpp"
#include "core/ScrollableList.hpp"
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cassert>
#include <conio.h>
#include <iostream>

PlayState::PlayState(App* app) :
	app(app),
	volumeReport("", core::Color::White),
	cooldownVolumeReport(),
	skipReport("", core::Color::Gray),
	cooldownSkipReport(),
	totalRunTime(0s),
	totalRunTimeClock(),
	playlist(),
	drawKeyInfo(true),
	playlistPath(),
	firstInit(true),
	style()
{
	
}

void PlayState::init()
{
	assert(!playlistPath.empty());

	if (firstInit) {
		app->messageBus.add(std::bind(&PlayState::onMessage, this, std::placeholders::_1));
		// This may not be called in the constructor, because App::messagebus is at this time not initialized.
		// Well, it could be if in the class declaration it is above PlayState declared, but this is ugly.
		// Note: I don't want to delete myself as as an receiver in termiante(), because I still want to
		// receive messages, so I check for 'wasOnceInitialized'.
	}

	style = app->style.playState;

	std::map<std::wstring, std::wstring> config = core::getConfig("data/config.dat");

	int options = (config[L"isPlaylistShuffled"] == L"true" ? core::Playlist::Shuffle : 0) | 
		          (config[L"playlistLoop"]       == L"true" ? core::Playlist::Loop : 0)    | 
		          core::Playlist::FadeOut |
	              core::Playlist::AutoStart;

	playlist.init(playlistPath, app->musicDirs, options);

	if (config[L"totalRuntime"] != L"nolimit") {
		totalRunTime = core::Seconds(std::stoi(config[L"totalRuntime"]));
	}

	totalRunTimeClock.restart();

	core::ScrollableList::InitInfo sliInfo;
	sliInfo.options = (core::ScrollableList::Options)((int)core::ScrollableList::Options::DrawCentered | (int)core::ScrollableList::Options::SelectionMode);
	sliInfo.style = app->style.scrollableList;
	sliInfo.name = "playlist";
	sliInfo.columnLayout = {};
	sliInfo.spaceBetweenColumns = 3;
	sliInfo.sizeInside = { 60, 18 };
	sliInfo.hover = 0;
	musicList.init(sliInfo);
	for (int i = 0; i < playlist.size(); ++i) {
		musicList.push_back({ playlist.at(i).title });
	}
	musicList.onConsoleResize();

	firstInit = false;
}

void PlayState::terminate()
{
	volumeReport = core::Text();
	skipReport = core::Text();
	totalRunTime = 0s;
	// drawKeyInfo can stay as is
	playlist.terminate();
	musicList.terminate();
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

	if (playlist.size() > 0) {
		// ..may not be called on empty playlists.
		musicList.select(playlist.getCurrentMusicIndex());
	}

	playlist.update();
	musicList.update();
}

void PlayState::handleEvent()
{
	using namespace core;

	if (core::inputDevice::isKeyPressed(VK_RETURN)) {
		musicList.selectHoveredItem();
		playlist.start(musicList.getSelectedIndex());
	}

	//ESC:
	if (inputDevice::isKeyPressed(VK_ESCAPE, false)) {
		// TODO Send message: Playlist stopped
		playlist.stop();
	}

	//B-Key:
	if (inputDevice::isKeyPressed('B'))
		app->messageBus.send(::Message::PlayState_Finished);

	//F12-Key:
	if (inputDevice::isKeyPressed(VK_F12, true))
		inputDevice::lock(!inputDevice::isLocked());

	// K-Key:
	if (inputDevice::isKeyPressed('K'))
		drawKeyInfo = !drawKeyInfo;

	// Up / Down key (or music finished):
	if (inputDevice::isKeyPressed(VK_UP)) {
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

	//E-Key:
	if (inputDevice::isKeyPressed('E'))
		playlist.setLoop(!playlist.getLoop());

	//L-Key
	if (inputDevice::isKeyPressed('L'))
		playlist.setCurrentMusicLoop(!playlist.getCurrentMusicLoop());

	//R-Key
	if (inputDevice::isKeyPressed('R')) {
		if (playlist.isShuffled()) playlist.resetShuffle();
		else playlist.shuffle();
	}

	//Right-Key:
	if (inputDevice::isKeyPressed(VK_RIGHT))
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

	//Left-Key:
	if (inputDevice::isKeyPressed(VK_LEFT))
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

	musicList.handleEvent();
}

intern void coutWithProgressBar(int& progressBarSize, std::string text, core::Color textColor, core::Color progressBarTextColor, core::Color progressBarColor)
{
	std::cout << core::Text(progressBarSize > text.size() ? text : text.substr(0, progressBarSize), progressBarSize > 0 ? progressBarTextColor : textColor, progressBarSize > 0 ? progressBarColor : core::Color::None)
		      << core::Text(progressBarSize > text.size() ? ""   : text.substr(progressBarSize, text.size() - progressBarSize), textColor, core::Color::None);
	progressBarSize -= text.size();
}

void PlayState::draw()
{
	// Back & list status:
	std::string listStatus = "";
	std::string backInfo = drawKeyInfo ? " <[B]ack"s : " <"; // If you use core::uc::leftwardsArrow, then note that std::string cannot properly handle unicode and its length is wrong.
	if (playlist.size() == 0)
		listStatus = "-";
	else if (playlist.getCurrentMusicNumber() <= playlist.size())
		listStatus = std::to_string(playlist.getCurrentMusicNumber());
	else listStatus = std::to_string(playlist.size());
	listStatus += " / " + (playlist.size() > 0 ? std::to_string(playlist.size()) : "-") + "";
	std::string skipForwardKeyInfo = drawKeyInfo ? "["s + core::uc::downwardsArrow + "] " : "";
	std::string skipBackwardKeyInfo = drawKeyInfo ? " ["s + core::uc::upwardsArrow + "]" : "";
	int listStatusKeyInfoLength = skipForwardKeyInfo.length() > 0 ? 8 : 0; // [^] ... [v]; note length is wrong because of unicode characters
	int drawPos = core::console::getCharCount().x / 2.f - (listStatus.length() + listStatusKeyInfoLength) / 2.f;
	drawPos -= backInfo.length();
	core::console::setBgColor(core::Color::Gray);
	std::cout << core::Text(backInfo, core::Color::White)
		<< core::Text(std::string(drawPos, ' '), core::Color::Black)
		<< core::Text(skipForwardKeyInfo, core::Color::White)
		<< core::Text(listStatus, core::Color::Bright_White)
		<< core::Text(skipBackwardKeyInfo, core::Color::White);
	// Lock events:
	core::Text inputInfo(drawKeyInfo ? "[F12] " : "", core::Color::White);
	if (core::inputDevice::isLocked())
		inputInfo.str += "Locked Input";
	if (!core::inputDevice::isLocked())
		inputInfo.str += "Free Input";
	drawPos = (core::console::getCharCount().x - core::console::getCursorPos().x) - inputInfo.str.length();
	std::cout << core::Text(std::string(drawPos - 1, ' '), core::Color::Black)
		<< inputInfo << core::endl();

	// title:
	std::string title = playlist.size() == 0 ? "-" : playlist.current().title;
	drawPos = core::console::getCharCount().x / 2.f - title.length() / 2.f;
	title.insert(0, drawPos, ' ');
	core::console::setBgColor(core::Color::Bright_White);
	std::cout << core::endl() << core::Text(title, core::Color::Black) << core::endl(2);

	// Duration:
	{
		std::string currDuration = playlist.size() == 0 ? "-" : getTimeStr(playlist.getCurrentMusicElapsedTime(), playlist.getCurrentMusicDuration());
		std::string duration = playlist.size() == 0 ? "-" : getTimeStr(playlist.getCurrentMusicDuration());
		std::string durationInfo = currDuration + skipReport.str + " of " + duration;
		std::string durationSkipForwardKeyInfo = drawKeyInfo ? "["s + core::uc::leftwardsArrow + "] " : "";
		std::string durationSkipBackwardKeyInfo = drawKeyInfo ? " ["s + core::uc::rightwardsArrow + "]" : "";
		int durationKeyInfoLength = durationSkipForwardKeyInfo.length() > 0 ? 8 : 0; // [<] ... [>]; note length is wrong because of unicode characters
		drawPos = core::console::getCharCount().x / 2.f - (durationInfo.length() + durationKeyInfoLength) / 2.f;
		skipReport.bgcolor = core::Color::White; // TODO set it in constructor
		core::console::setBgColor(core::Color::White);

		float progressBarFactor = playlist.size() == 0 ? 0 : playlist.getCurrentMusicElapsedTime().asSeconds() / playlist.getCurrentMusicDuration().asSeconds(); // 0..duration
		int progressBarSize = core::console::getCharCount().x * progressBarFactor; // 0..consoleCharCountX

		std::string spaceBefore(drawPos, ' ');
		std::string spaceAfter; // can not be initialized here because of getCursorPos()
		coutWithProgressBar(progressBarSize, spaceBefore,                 style.durationText, style.durationProgressBarText, style.durationProgressBar);
		coutWithProgressBar(progressBarSize, durationSkipForwardKeyInfo,  core::Color::Gray, style.durationProgressBarText, style.durationProgressBar);
		coutWithProgressBar(progressBarSize, currDuration,                style.durationText, style.durationProgressBarText, style.durationProgressBar);
		coutWithProgressBar(progressBarSize, skipReport.str,              skipReport.fgcolor, skipReport.bgcolor, style.durationProgressBar);
		coutWithProgressBar(progressBarSize, " of ",                      style.durationText, style.durationProgressBarText, style.durationProgressBar);
		coutWithProgressBar(progressBarSize, duration,                    style.durationText, style.durationProgressBarText, style.durationProgressBar);
		coutWithProgressBar(progressBarSize, durationSkipBackwardKeyInfo, core::Color::Gray, style.durationProgressBarText, style.durationProgressBar);
		spaceAfter = std::string(core::console::getCharCount().x - (core::console::getCursorPos().x + 1), ' ');
		coutWithProgressBar(progressBarSize, spaceAfter,                  style.durationText, style.durationProgressBarText, style.durationProgressBar);
		std::cout << core::endl();

		core::console::setBgColor(core::Color::Black); //app->style.bgcolor
		std::cout << core::endl();
	}

	core::console::setFgColor(style.text);

	if (playlist.size() > 0)
	{
		// artist:
		std::cout << std::string(musicList.getPosX(), ' ') << "Artist:         " << playlist.current().artist << core::endl();

		// album:
		std::cout << std::string(musicList.getPosX(), ' ') << "Album:          " << playlist.current().album << core::endl();

		// volume:			          
		std::cout << std::string(musicList.getPosX(), ' ') << "Volume:         " << playlist.getVolume() << "% " << volumeReport
			<< core::Text(drawKeyInfo ? " [+/-]" : "", core::Color::Gray) << core::endl();

		// state:
		std::cout << std::string(musicList.getPosX(), ' ');
		if (playlist.isPaused())
			std::cout << "State:          Paused" << core::Text(drawKeyInfo ? " [(P)lay]" : "", core::Color::Gray);
		else if (playlist.isPlaying())
			std::cout << "State:          Playing" << core::Text(drawKeyInfo ? " [(P)ause]" : "", core::Color::Gray);
		else if (playlist.isStopped() || playlist.isCurrentMusicStopped())
			std::cout << "State:          Stopped";
		if (playlist.getCurrentMusicLoop())
			std::cout << " [loop]";
		else
		{
			if (playlist.current().playCount == 1)
				std::cout << " [" << core::Text("1", core::Color::Bright_White) << " time]";
			else std::cout << " [" << core::Text(std::to_string(playlist.current().playCount), core::Color::Bright_White) << " times]";
		}
		std::cout << core::Text(drawKeyInfo ? " [W/S; (L)oop]" : "", core::Color::Gray) << core::endl();

		// Is shuffled?:
		std::cout << std::string(musicList.getPosX(), ' ');
		if (playlist.isShuffled()) std::cout << "Shuffled:       yes";
		else std::cout << "Shuffled:       no";
		std::cout << core::Text(drawKeyInfo ? " [R]" : "", core::Color::Gray) << core::endl();

		// playlist state:
		std::cout << std::string(musicList.getPosX(), ' ');
		if (playlist.getLoop()) std::cout << "Playlist State: endless";
		else std::cout << "Playlist State: not endless";
		std::cout << core::Text(drawKeyInfo ? " [E]" : "", core::Color::Gray) << core::endl();

		// total time:
		std::cout << std::string(musicList.getPosX(), ' ');
		if (totalRunTime.asSeconds() == 0.f)
			std::cout << "Total runtime:  no limit";
		else
		{
			std::cout << "Total runtime:  ";
			if (totalRunTimeClock.getElapsedTime().asSeconds() < 60.f)
				std::cout << static_cast<unsigned short>(totalRunTimeClock.getElapsedTime().asSeconds()) << core::Text("sec", core::Color::Gray) << " von ";
			else std::cout << totalRunTimeClock.getElapsedTime().asMinutes() << core::Text("min", core::Color::Gray) << " von ";
			if (totalRunTime.asSeconds() < 60.f)
				std::cout << static_cast<unsigned short>(totalRunTime.asSeconds()) << core::Text("sec", core::Color::Gray);
			else std::cout << totalRunTime.asSeconds() / 60.f << core::Text("min", core::Color::Gray);
		}
		std::cout << core::endl() << core::endl();
	}
	else {
		std::cout << std::string(musicList.getPosX(), ' ') << core::Text("There is no music!", core::Color::Bright_White) << core::endl();
		std::cout << std::string(musicList.getPosX(), ' ') << "Playlist is empty or music could not be found!" << core::endl()
			<< std::string(musicList.getPosX(), ' ') << "Searched for music in:" << core::endl();
		for (auto& musicDir : app->musicDirs) {
			std::wcout << std::wstring(musicList.getPosX(), ' ') << L"- " << musicDir.wstring() << core::endl();
		}
		std::cout << core::endl();
	}

	musicList.draw();

	// Footer:
	int lineCount = core::console::getCharCount().y;
	for (int i = core::console::getCursorPos().y; i < lineCount - 1; ++i) {
		std::cout << core::endl();
	}
	std::cout << core::Text(drawKeyInfo ? " Hide [K]ey Info" : " Show [K]ey Info", core::Color::Gray, core::Color::White);
	std::cout << core::Text(std::string(core::console::getCharCount().x - core::console::getCursorPos().x, ' '), core::Color::Gray, core::Color::White);
}

void PlayState::onMessage(core::Message message)
{
	if (message.id == core::MessageID::CONSOLE_RESIZE) {
		musicList.onConsoleResize();
	}
}

void PlayState::setPlaylist(fs::path playlistPath)
{
	this->playlistPath = playlistPath;
}