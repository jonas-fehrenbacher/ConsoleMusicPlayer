#include "App.hpp"
#include "Tools/InputDevice.hpp"
#include "Tools/Tool.hpp"
#include "Message/Messages.hpp"
#include <fstream>
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

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

	stateMachine.add(&menuState);
	messageBus.add(std::bind(&App::onMessage, this, std::placeholders::_1));

	// Clear log file:
	std::ofstream ofs("data/log.txt");
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