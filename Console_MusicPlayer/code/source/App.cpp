#include "App.hpp"
#include "Tools/InputDevice.hpp"
#include "Tools/Tool.hpp"
#include "Message/Messages.hpp"
#include <fstream>
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <SDL.h>
#include <SDL_mixer.h>
namespace fs = std::filesystem;

// Debug:
double wavGetSeconds(const char* filename) {
	// See: https://stackoverflow.com/questions/76030221/is-it-possible-to-get-length-in-seconds-of-a-loaded-wav-file-in-sdl-library
	SDL_AudioSpec spec;
	uint32_t audioLen;
	uint8_t* audioBuf;
	double seconds = 0.0;
	
	if (SDL_LoadWAV(filename, &spec, &audioBuf, &audioLen) != NULL) {
		// we aren't using the actual audio in this example
		SDL_FreeWAV(audioBuf);
		uint32_t sampleSize = SDL_AUDIO_BITSIZE(spec.format) / 8;
		uint32_t sampleCount = audioLen / sampleSize;
		// could do a sanity check and make sure (audioLen % sampleSize) is 0
		uint32_t sampleLen = 0;
		if (spec.channels) {
			sampleLen = sampleCount / spec.channels;
		}
		else {
			// spec.channels *should* be 1 or higher, but just in case
			sampleLen = sampleCount;
		}
		seconds = (double)sampleLen / (double)spec.freq;
	}
	else {
		// uh-oh!
		fprintf(stderr, "ERROR: can't load: %s: %s\n", filename, SDL_GetError());
	}

	return seconds;
}

App::App() :
	isRunning(true),
	stateMachine(),
	menuState(this),
	playState(this),
	playlistEditorState(this),
	drawTimer(),
	messageBus()
{
	SetConsoleTitle("Music Player");
	core::setWindowSize(70, 80); // 50 60
	core::setWindowPos(0, 0);
	srand(time(nullptr));

	// Init SDL2:
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) { // enables to invoke SDL2 functions
		std::cout << "SDL initialization failed! SDL Error: " << SDL_GetError() << "\n";
		__debugbreak();
	}

	// Init SDL_mixer:
	// IX_INIT_FLAC | MIX_INIT_MOD | MIX_INIT_MP3 | MIX_INIT_OGG | MIX_INIT_MID | MIX_INIT_OPUS
	if (Mix_Init(MIX_INIT_FLAC | MIX_INIT_MOD | MIX_INIT_MP3 | MIX_INIT_OGG | MIX_INIT_MID | MIX_INIT_OPUS) == 0) {
		std::cout << "SDL mixer initialization failed! SDL Error: " << Mix_GetError() << "\n";
		__debugbreak();
	}
	int frequency{ 44100 }; // 44.1KHz, which is CD audio rate (Most games use 22050, because 44100 requires too much CPU power on older computers)
	int hardware_channels{ 2 }; // 2 for stereo
	int chunksize{ 4096 }; // 2048
	if (Mix_OpenAudio(frequency, MIX_DEFAULT_FORMAT, hardware_channels, chunksize) == -1) {
		std::cout << "SDL mixer initialization failed! SDL Error: " << Mix_GetError() << "\n";
		__debugbreak();
	}
	
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

	if (message == Message::MenuState_SelectedPlaylistEditor) {
		stateMachine.remove(&menuState);
		stateMachine.add(&playlistEditorState);
	}

	if (message == Message::PlayState_Finished) {
		stateMachine.remove(&playState);
		stateMachine.add(&menuState);
	}

	if (message == Message::PlaylistEditorState_Finished) {
		stateMachine.remove(&playlistEditorState);
		stateMachine.add(&menuState);
	}
}