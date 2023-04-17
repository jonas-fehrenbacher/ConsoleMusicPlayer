#include "App.hpp"
#include "Tools/InputDevice.hpp"
#include "Tools/Tool.hpp"
#include "Message/Messages.hpp"
#include <fstream>
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
namespace fs = std::filesystem;

App::App() :
	isRunning(true),
	stateMachine(),
	menuState(this),
	playState(this),
	drawTimer(),
	messageBus()
{
	SetConsoleTitle("Music Player");
	core::setWindowSize(50, 60);
	core::setWindowPos(0, 0);
	srand(time(nullptr));

	// Clear log file:
	std::ofstream ofs("data/log.txt");
	ofs.close();
	// Create music/:
	if (!fs::exists("music/")) {
		fs::create_directories("music/");
	}
	// Create data/:
	if (!fs::exists("data/")) {
		fs::create_directories("data/");
	}
	// Create data/config.dat:
	if (!fs::exists("data/config.dat")) {
		ofs.open("data/config.dat");
		ofs << "defaultPlaylist = 2\nisPlaylistShuffled = true\nplaylistLoop = true\ntotalRuntime = nolimit";
		ofs.close();
	}

	// ..after folder and files are created:
	stateMachine.add(&menuState);
	messageBus.add(std::bind(&App::onMessage, this, std::placeholders::_1));
}

void App::handleEvents()
{
	stateMachine.handleEvent();
	if (core::inputDevice::isKeyPressed(VK_ESCAPE)) {
		isRunning = false;
	}
}

void App::mainLoop()
{
	while (isRunning)
	{
		handleEvents();

		stateMachine.update();
		messageBus.update();

		if (drawTimer.getElapsedTime() >= 500ms) {
			core::clearScreen();
			stateMachine.draw();
			drawTimer.restart();
		}
	}
}

void App::onMessage(int message)
{
	if (message == Message::MenuState_EnteredPlaylist) {
		stateMachine.remove(&menuState);
		stateMachine.add(&playState);
	}

	if (message == Message::PlayState_Finished) {
		stateMachine.remove(&playState);
		stateMachine.add(&menuState);
	}
}