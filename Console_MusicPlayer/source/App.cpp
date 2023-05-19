#include "App.hpp"
#include "Messages.hpp"
#include "core/InputDevice.hpp"
#include "core/SmallTools.hpp"
#include "core/Console.hpp"
#include "core/Profiler.hpp"
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <fstream>
#include <SDL.h>
#include <SDL_mixer.h>
#include <codecvt>
#include <locale>
#include <iostream>
#include <thread>

intern std::vector<fs::path> getMusicDirsFromConfig();
intern App::Style getStyle();
intern void loadingScreen(std::atomic_bool* isInitializing, App::LoadingScreenStyle style);
App::App() :
	messageBus(),
	isRunning(true),
	activeState(nullptr),
	trackState(this),
	playlistState(this),
	directoryState(this),
	drawTimer(),
	musicDirs(), // do not initialize here, because maybe config.dat does not exist.
	style(),
	isDrawKeyInfo(true)
{
	///////////////////////////////////////////////////////////////////////////////
	// Setup console
	///////////////////////////////////////////////////////////////////////////////
	core::console::init();
	core::console::setTitle("Console Music Player");
	core::console::setSize(90, 35, true); // 1000, 900
	//core::console::setPos(325, 100);
	core::console::hideCursor();
	srand(time(nullptr));
	// Setup unicode console:
	// See: https://stackoverflow.com/questions/2492077/output-unicode-strings-in-windows-console-app
	std::ios_base::sync_with_stdio(false); // maybe optional
	std::locale utf8(std::locale(), new std::codecvt_utf8_utf16<wchar_t>);
	std::wcout.imbue(utf8); // Required to output utf-8 paths.
	SetConsoleOutputCP(CP_UTF8); // required
	SetConsoleCP(CP_UTF8); // optional
	char* a = setlocale(LC_ALL, ".UTF8");
	//SetConsoleOutputCP(437);
	core::console::setFont(L"Consolas", 24); // Consolas, core::console::DEFAULT_UNICODE_FONTNAME
	/* Good fonts:
	 * - Consolas
	 * - DejaVu Sans Mono (some unicode)
	 * - Source Code Pro Semibold
	 */
	// Set style:
	// ..set this before the loading screen!
	style = getStyle();
	core::console::setFgColor(style.default.fg);
	core::console::setBgColor(style.default.bg);

	///////////////////////////////////////////////////////////////////////////////
	// Loading screen
	///////////////////////////////////////////////////////////////////////////////
	std::atomic_bool isInitializing = true;
	std::thread loadingThread(loadingScreen, &isInitializing, style.loadingScreen);

	///////////////////////////////////////////////////////////////////////////////
	// Init SDL2
	///////////////////////////////////////////////////////////////////////////////
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
	// Create SDL2 window
	// This is required to receive SDL_Event's.
	//SDL_WINDOW_HIDDEN | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS | SDL_WINDOW_KEYBOARD_GRABBED | SDL_WINDOW_MOUSE_GRABBED
	//SDL_Window* window = NULL;
	//window = SDL_CreateWindow("CMP", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 500, 500, 0);
	//if (window == NULL) {
	//	core::log("Window could not be created! SDL_Error: "s + SDL_GetError());
	//	__debugbreak();
	//	exit(1);
	//}

	///////////////////////////////////////////////////////////////////////////////
	// Init events
	///////////////////////////////////////////////////////////////////////////////
	core::inputDevice::init(std::bind(&App::terminate, this), &messageBus);
	
	///////////////////////////////////////////////////////////////////////////////
	// Setup files
	///////////////////////////////////////////////////////////////////////////////
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
		std::string username = core::getUsername();
		fs::path defaultMusicDir = "C:/Users/" + username + "/Music";
		if (username.empty() || !fs::exists(defaultMusicDir)) {
			defaultMusicDir = "";
		}
		ofs.open("data/config.dat");
		ofs << "defaultPlaylist = 0\nisPlaylistShuffled = false\nmusicDirs = " << defaultMusicDir << "\nplaylistLoop = none\ntotalRuntime = nolimit";
		ofs.close();
		// "D:/Data/Music/", "C:/Users/Jonas/Music/", "music/"
	}

	///////////////////////////////////////////////////////////////////////////////
	// Init music dirs
	///////////////////////////////////////////////////////////////////////////////
	// Set music directories:
	// Note set this after config.dat is created and before State::init() is called.
	musicDirs = getMusicDirsFromConfig();

	///////////////////////////////////////////////////////////////////////////////
	// Get config
	///////////////////////////////////////////////////////////////////////////////
	// Do not do this in the constructor, because in App::App I need to check if the directory exists.
	std::map<std::wstring, std::wstring> config = core::getConfig("data/config.dat");
	if (config.count(L"defaultPlaylist") == 0) {
		core::log("Error: defaultPlaylist not found in config.dat");
		__debugbreak();
	}

	///////////////////////////////////////////////////////////////////////////////
	// Init music player
	///////////////////////////////////////////////////////////////////////////////
	core::Time musicPlayer_sleepTime = 0s;
	if (config[L"totalRuntime"] != L"nolimit") {
		musicPlayer_sleepTime = core::Seconds(std::stoi(config[L"totalRuntime"]));
	}
	int musicPlayer_options =
		(config[L"isPlaylistShuffled"] == L"true" ? core::MusicPlayer::Shuffle : 0) |
		(config[L"playlistLoop"] == L"none" ? 0 : (config[L"playlistLoop"] == L"one" ? core::MusicPlayer::LoopOne : core::MusicPlayer::LoopAll)) |
		core::MusicPlayer::FadeOut;
	musicPlayer.init(musicDirs, style.drawableList, musicPlayer_options, musicPlayer_sleepTime);

	///////////////////////////////////////////////////////////////////////////////
	// Init rest
	///////////////////////////////////////////////////////////////////////////////
	messageBus.add(std::bind(&App::onMessage, this, std::placeholders::_1));
	title.init(this);
	navBar.init(this);
	playStatus.init(this);
	footer.init(this);

	///////////////////////////////////////////////////////////////////////////////
	// Init states
	///////////////////////////////////////////////////////////////////////////////
	// ..after folder and files are created:
	activeState = &trackState;
	activeState->init();

	// Wait for thread:
	//std::this_thread::sleep_for(10s);
	isInitializing = false;
	loadingThread.join(); // wait for the thread.
}

App::Style getStyle()
{
	App::Style style;

	// Set app style:
	style.default = { core::Color::White, core::Color::Black };
	// style.loadingScreen:
	style.loadingScreen.background      = core::Color::White;
	style.loadingScreen.title           = core::Color::Black;
	style.loadingScreen.loadingText     = core::Color::Gray;
	style.loadingScreen.loadingTextAnim = core::Color::Bright_White;
	// style.scrollableList:
	style.drawableList.border              = core::Color::Gray;
	style.drawableList.title               = core::Color::Bright_White;
	style.drawableList.scrollbarArrow      = core::Color::Aqua;
	style.drawableList.scrollbar           = core::Color::Aqua;
	style.drawableList.scrollbarEmptySpace = core::Color::White;
	style.drawableList.item                = core::Color::White;
	style.drawableList.borderItem          = core::Color::Gray;
	style.drawableList.selected            = core::Color::Green;
	style.drawableList.hover               = core::Color::Bright_White;
	// Title:
	style.title.background  = core::Color::Bright_White;
	style.title.title       = core::Color::Black;
	style.title.trackNumber = core::Color::Black;
	style.title.keyInfo     = core::Color::White;
	style.title.exitSymbol  = core::Color::Gray;
	style.title.lockStatus  = core::Color::Gray;
	// NavBar:
	style.navBar.background    = core::Color::Bright_White;
	style.navBar.keyInfo       = core::Color::White;
	style.navBar.item          = { core::Color::Black, core::Color::None };
	style.navBar.item_hover    = { core::Color::Black, core::Color::White };
	style.navBar.item_selected = { core::Color::Bright_White, core::Color::Gray };
	// PlayStatus:
	style.playStatus.statusOn                = core::Color::Green;
	style.playStatus.statusOff               = core::Color::Gray;
	style.playStatus.durationProgressBar     = core::Color::White;
	style.playStatus.durationProgressBarText = core::Color::Black;
	style.playStatus.durationText            = core::Color::Black;
	// Footer:
	style.footer.background      = core::Color::White;
	style.footer.keyShortcut     = { core::Color::Bright_White, core::Color::Aqua };
	style.footer.keyShortcutText = core::Color::Gray;

	return style;
}

intern void loadingScreen(std::atomic_bool* isInitializing, App::LoadingScreenStyle style)
{
	// ..is called in an separate thread
	core::console::setBgColor(style.background);
	std::string title = "Console Music Player "s + core::uc::eighthNote;
	std::string loadingText = "Loading music, please wait...";
	std::string rotatingSlash = u8"\\|/\u2500"s + core::uc::boxDrawingsLightHorizontal;
	int rotatingSlashIndex = 0;
	int highlightIndex = 0; // for animation
	core::Timer animTimer;
	bool isHighlightAnimFinished = false;
	while ( *isInitializing)
	{
		if (!isHighlightAnimFinished && animTimer.getElapsedTime() >= 200ms && highlightIndex < loadingText.length()) {
			++highlightIndex;
			animTimer.restart();
			isHighlightAnimFinished = highlightIndex == loadingText.length();
			if (isHighlightAnimFinished) --highlightIndex;
		}

		if (isHighlightAnimFinished && animTimer.getElapsedTime() >= 150ms) {
			rotatingSlashIndex = ++rotatingSlashIndex % 4;
			animTimer.restart();
		}

		int titlePosX = core::console::getCharCount().x / 2.f - (title.length() - 2) / 2.f;// -2 because std::string cannot handle unicode characters - the length is to much.
		int loadingPosX = core::console::getCharCount().x / 2.f - (loadingText.length() - 2) / 2.f;
		int posY = core::console::getCharCount().y / 2.f - 2;
		// Draw:
		std::cout << core::endl(posY)
			// title:
			<< std::string(titlePosX, ' ') << core::Text(title, style.title) << core::endl()
			// loading text:
			<< std::string(loadingPosX, ' ') 
			<< core::Text(loadingText.substr(0, highlightIndex), style.loadingText) 
			<< core::Text(""s + loadingText.at(highlightIndex), isHighlightAnimFinished ? style.loadingText : style.loadingTextAnim) // draw only until highlightIndex - is more interesting and nice then drawing everything.
			<< core::endl();

		if (isHighlightAnimFinished) {
			// ..start next animation
			// Note that unicode character takes up 3 characters instead of 1.
			std::cout << std::string(core::console::getCharCount().x / 2.f, ' ') << core::Text(rotatingSlash.substr(rotatingSlashIndex, rotatingSlashIndex == 3 ? 3 : 1), style.loadingTextAnim);
		}

		core::console::clearScreen();
	}
}

intern std::vector<fs::path> getMusicDirsFromConfig()
{
	// Set music directories: 
	// std::filesystem::weakly_canonical(): convert to absolute path that has no dot, dot-dot elements or symbolic links in its generic format representation.
	// std::mismatch(begin1, end1, begin2): 
	// - end2 is equal begin2 + (end1-begin1)
	// - Returns iterator to the first mismatch [it1, it2] or if it is equal then [end1+1, end2].
	// - Example:
	//   const fs::path path1 = "C:/user/jonas/music/test", path2 = "C:/user/jonas/music/Loblieder";
	//   auto [it1, it2] = std::mismatch(path1.begin(), path1.end(), path2.begin());
	//   std::cout << "it1: " << *it1 << ", it2: " << *it2 << "\n"; // it1: "test", it2: "Loblieder"
	// - IMPORTANT: This only works if there is no trailing slash (/).
	//   std::path::iterator iterates over each directory and with an trailing slash there is probably an additional entry.
	std::vector<fs::path> musicDirs = core::getConfigPathArr(core::getConfig("data/config.dat")[L"musicDirs"]);
	// Erase all no-directory entries:
	for (auto it = musicDirs.begin(); it != musicDirs.end();) {
		if (!fs::exists(*it) || !fs::is_directory(*it)) it = musicDirs.erase(it);
		else ++it;
	}
	// Erase all subdirectory entries:
	// IMPORTANT: std::mismatch only works if there is no trailing slash (/), so we remove it first.
	for (auto& musicDir : musicDirs) { // (reference is required)
		if (musicDir.u8string().back() == '/' || musicDir.u8string().back() == '\\') { // important: needs to be u8string() for unicode paths.
			std::wstring normalizedPath = musicDir.wstring();
			normalizedPath.pop_back();
			musicDir = normalizedPath;
		}
	}
	for (auto subPathIt = musicDirs.begin(); subPathIt != musicDirs.end();) {
		fs::path subPath = std::filesystem::weakly_canonical(*subPathIt);
		bool isSubpath = false;
		for (auto rootPathIt = musicDirs.begin(); rootPathIt != musicDirs.end(); ++rootPathIt) {
			if (subPathIt == rootPathIt) {
				// ..is same directory, skip.
				continue;
			}
			fs::path rootPath = std::filesystem::weakly_canonical(*rootPathIt);
			auto [rootIT, subIT] = std::mismatch(rootPath.begin(), rootPath.end(), subPath.begin());
			if (rootIT == rootPath.end()) {
				// ..rootPath is really a root path of subPath
				// To avoid adding music twice we have to delete the sub path.
				subPathIt = musicDirs.erase(subPathIt);
				isSubpath = true;
				break;
			}
		}
		if (!isSubpath) {
			++subPathIt;
		}
	}

	// Set default music directory:
	// Should not be added automatically, because user should control what he wants. I add this only if config.dat does not exist.
	//std::string username = core::getUsername();
	//if (!username.empty()) {
	//	fs::path defaultMusicDir = "C:/Users/" + username + "/Music";
	//	if (fs::exists(defaultMusicDir)) {
	//		bool isAlreadySet = false;
	//		for (auto& musicDir : musicDirs) {
	//			if (musicDir == defaultMusicDir) {
	//				isAlreadySet = true;
	//				break;
	//			}
	//		}
	//		if (!isAlreadySet) {
	//			musicDirs.push_back(defaultMusicDir);
	//		}
	//	}
	//}

	return musicDirs;
}

void App::terminate()
{
	core::console::reset();
	core::logProfiles("data/profiler.log");
	musicPlayer.terminate();
	// Terminating SDL takes to much time (console freezes) and isn't neccessary if program is killed anyway:
	//Mix_Quit();
	//SDL_Quit();
}

void App::mainLoop()
{
	while (isRunning)
	{
		handleEvents();

		update();

		if (drawTimer.getElapsedTime() >= 250ms) {
			// The more you draw, the less frequent handleEvent is called and it happens that keys are missed (maybe use somehow SDL...)
			core::console::clearScreen();
			draw();
			drawTimer.restart();
		}
	}

	// Terminate:
	terminate();
}

void App::update()
{
	activeState->update();
	messageBus.update();
	// maybe update musicDirs (optional)
	core::inputDevice::update();
	musicPlayer.update();
}

void App::handleEvents()
{
	///////////////////////////////////////////////////////////////////////////////
	// Exit
	///////////////////////////////////////////////////////////////////////////////
	if (core::inputDevice::isKeyPressed(VK_ESCAPE)) {
		isRunning = false;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Lock
	///////////////////////////////////////////////////////////////////////////////
	if (core::inputDevice::isKeyPressed(VK_F12, true)) {
		core::inputDevice::lock(!core::inputDevice::isLocked());
	}

	///////////////////////////////////////////////////////////////////////////////
	// Hide key
	///////////////////////////////////////////////////////////////////////////////
	if (core::inputDevice::isKeyPressed('K')) {
		isDrawKeyInfo = !isDrawKeyInfo;
	}

	///////////////////////////////////////////////////////////////////////////////
	// States
	///////////////////////////////////////////////////////////////////////////////
	activeState->handleEvent();

	///////////////////////////////////////////////////////////////////////////////
	// Music player
	///////////////////////////////////////////////////////////////////////////////
	musicPlayer.handleEvents();
}

void App::draw()
{
	activeState->draw();
	//musicPlayer.draw();
}

void App::onMessage(core::Message message)
{
	if (message.id == core::MessageID::CONSOLE_RESIZE) {
		core::console::hideCursor(); // is required, because somehow cursor is shown again after reset
		musicPlayer.onConsoleResize();
	}

	if (message.id == Message::NAVBAR_OPTION_SELECTED) {
		NavBar::Option option = *static_cast<NavBar::Option*>(message.userData);
		activeState->terminate();
		if (option == NavBar::Option::AllMusic)         activeState = &trackState;
		else if (option == NavBar::Option::Playlists)   activeState = &playlistState;
		else if (option == NavBar::Option::Directories) activeState = &directoryState;
		activeState->init();
	}
}