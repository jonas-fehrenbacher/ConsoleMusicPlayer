#include "Music/Playlist.hpp"
#include "Tools/Tool.hpp"
#include <filesystem>
#include <fstream>
#include <random>
#include <iostream>
#include <cassert>

core::Playlist::Playlist() :
	playlist({}),
	name(""),
	current_(0), // must be 0 (see shuffle)
	loop(false),
	music(),
	playtime(),
	oldElapsedTime(0s),
	fadeOutActive(false),
	volume(100), // default
	fadeOutEnabled(false),
	currentMusicLoop(false)
{
	
}

void core::Playlist::addNewEntry(std::filesystem::path path)
{
	if (!fs::exists(path)) {
		log("File (" + path.string() + ") not found!\n");
		return;
	}

	// Is music file?:
	if (!isSupportedAudioFile(path)) {
		return;
	}

	// Open music to get its metadata:
	Mix_Music* music = Mix_LoadMUS(path.make_preferred().u8string().c_str());
	if (!music) {
		log("Failed to load music! SDL_mixer Error: " + std::string(Mix_GetError()) + "\n");
		//__debugbreak();
		return;
	}

	std::string filenameStem = path.stem().u8string();
	Entry entry;
	entry.path = path.make_preferred().wstring();
	std::string sdlTitle = Mix_GetMusicTitle(music);
	entry.title = strcmp(Mix_GetMusicTitle(music), "") == 0? filenameStem : Mix_GetMusicTitle(music); // SDL2 does not return filename as mentioned, so I do it manually.
	entry.artist = strcmp(Mix_GetMusicArtistTag(music), "") == 0 ? "unknown" : Mix_GetMusicTitle(music);
	entry.album = strcmp(Mix_GetMusicAlbumTag(music), "") == 0 ? "unknown" : Mix_GetMusicTitle(music);
	entry.duration = Time(Seconds((int)Mix_MusicDuration(music))); // IMPORTANT!: needs to be set once. IF this is called frequently, then the played music stutters!!!
	entry.playCount = 1;
	playlist.push_back(entry);
	Mix_FreeMusic(music);
}

void core::Playlist::shuffle()
{
	static std::random_device rd;
	static auto rng = std::mt19937(rd());
	if (isCurrentMusicStopped()) {
		// No music is playing or paused, so shuffle everything.
		std::shuffle(std::begin(playlist), std::end(playlist), rng);
	}
	else if (current_ < playlist.size() - 2) {
		// Shuffle only remaining music and current music may not be shuffled as well.
		// Start shuffling only if there are at least two pieces (0 |1| 2 3). 
		std::shuffle(std::begin(playlist) + current_ + 1, std::end(playlist), rng);
	}
}

static std::vector<fs::path> getMusicDirsFromConfig()
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

void core::Playlist::init(std::filesystem::path playlistPath, int options /*= 0*/)
{
	if (playlistPath.empty() || playlistPath == "data/all.pl") {
		init(options);
		return;
	}
	else if (!fs::exists(playlistPath)) {
		std::cout << "Error: Playlist (" << playlistPath << ") not found!\n";
		system("PAUSE");
	}

	musicDirs = getMusicDirsFromConfig();

	name = playlistPath.filename().string();
	std::string   filename;
	std::ifstream ifs(playlistPath, std::ios::in);

	while (std::getline(ifs, filename)) {
		// Playlist should only hold filenames, so that music files can be moved without editing the playlist file.
		// Search for music:
		for (auto& musicDir : musicDirs) {
			for (auto& it : fs::recursive_directory_iterator(musicDir)) {
				if (it.path().filename() == filename) {
					addNewEntry(it.path().wstring()); // IMPORTANT use here u8string to support Dvořák, but for something like 音楽 you need wstring.
					// Note: we iterate further, because in the other directory could be a file with the same name.
				}
			}
		}
	}

	ifs.close();

	_init(options);
}

void core::Playlist::init(int options /*= 0*/)
{
	// Set music directories:
	musicDirs = getMusicDirsFromConfig();

	for (auto& musicDir : musicDirs) {
		if (fs::exists(musicDir)) {
			for (auto& it : fs::recursive_directory_iterator(musicDir)) {
				addNewEntry(it.path().wstring()); // IMPORTANT use here u8string to support Dvořák, but for something like 音楽 you need wstring.
			}
		}
		else {
			log("Error: Music directory '" + musicDir.string() + "' could not be found!\n");
		}
	}
	_init(options);
}

void core::Playlist::_init(int options)
{
	// Options:
	if (hasFlag(Shuffle, options)) {
		shuffle();
	}
	if (hasFlag(Loop, options)) {
		loop = true;
	}
	if (hasFlag(FadeOut, options)) {
		fadeOutEnabled = true;
	}

	// Play:
	if (playlist.size() > 0 && fs::exists(current().path)) {
		music = Mix_LoadMUS(current().path.u8string().c_str()); // u8string() required for unicode path
		if (!music) {
			log("Failed to load music! SDL_mixer Error: " + std::string(Mix_GetError()) + "\n");
			__debugbreak();
		}
		Mix_PlayMusic(music, 0);
	}
	playtime.restart();
}

void core::Playlist::terminate()
{
	playlist.clear();
	name = "";
	current_ = 0;
	loop = false;
	Mix_HaltMusic();
	Mix_FreeMusic(music);
	oldElapsedTime = 0s;
	fadeOutEnabled = false;
	fadeOutActive = false;
	volume = 100;
}

void core::Playlist::play(bool next)
{
	if (playlist.empty()) {
		return;
	}

	if (current().playCount == 1)
	{
		// ..play next or previous music
		
		// Set current index (loop around on endless mode):
		if (next) {
			if (loop && current_ == playlist.size() - 1)
				current_ = 0;
			else ++current_;
		}
		else {
			if (loop && current_ == 0)
				current_ = playlist.size() - 1;
			else --current_;
		}

		// Play:
		if (current_ >= 0 && current_ < playlist.size()) {
			Mix_FreeMusic(music);
			music = Mix_LoadMUS(current().path.u8string().c_str()); // u8string() required for unicode path
			if (!music) {
				log("Failed to load music! SDL_mixer Error: " + std::string(Mix_GetError()) + "\n");
				__debugbreak();
			}
			Mix_PlayMusic(music, 0);
			currentMusicLoop = false;
			playtime.restart();
		}
		else {
			// ..playlist end reached
			playtime.restart();
			playtime.stop();
			// Music should stop on its own.
		}
	}
	else
	{
		// ..replay current music
		--current().playCount;
		Mix_PlayMusic(music, 0);
		currentMusicLoop = false;
		playtime.restart();
	}
}

void core::Playlist::playNext()
{
	play(true);
}

void core::Playlist::playPrevious()
{
	play(false);
}

void core::Playlist::update()
{
	if (playlist.empty()) {
		return;
	}

	// Loop:
	// playtime.getElapsedTime().asSeconds() >= currentMusicDuration().asSeconds() is not sure if it will be reached
	if (currentMusicGetLoop()) {
		if (oldElapsedTime > playtime.getElapsedTime()) {
			playtime.restart();
		}
		oldElapsedTime = playtime.getElapsedTime();
	}
	else oldElapsedTime = 0ns;

	// Fade out:
	if (fadeOutEnabled) {
		Time remainingPlaytime = currentMusicDuration() - playtime.getElapsedTime();
		Time fadeOutTime = 10s;
		float minFadeOutFactor = 0.2; // Otherwise I get error messages.
		if (remainingPlaytime <= fadeOutTime) {
			fadeOutActive = true;

			float fadeFactor = remainingPlaytime.asSeconds() / fadeOutTime.asSeconds(); // value from 0..1 (8sec / 10sec = 0.8)
			fadeFactor = fadeFactor < minFadeOutFactor ? minFadeOutFactor : fadeFactor;
			Mix_VolumeMusic(volume * fadeFactor); // important do not call this->setVolume() here.
		}
		else if (fadeOutActive && remainingPlaytime.asSeconds() > 10.f) {
			fadeOutActive = false;
			setVolume(volume);
		}
	}
}

void core::Playlist::resume()
{
	Mix_ResumeMusic();
	playtime.resume();
}

void core::Playlist::pause()
{
	Mix_PauseMusic();
	playtime.stop();
}

void core::Playlist::stop()
{
	Mix_HaltMusic();
	playtime.stop();
}

void core::Playlist::setVolume(float volume)
{
	Mix_VolumeMusic(volume);
	this->volume = volume;
}

void core::Playlist::skipTime(Time skipTime)
{
	Time time = playtime.getElapsedTime() + skipTime;
	if (time < 0s) {
		if (Mix_SetMusicPosition(0) == -1) {
			log("Mix_SetMusicPosition failed (" + current().path.stem().string() + ")!");
		}
		playtime.restart();
	}
	else {
		if (Mix_SetMusicPosition(time.asSeconds()) == -1) {
			log("Mix_SetMusicPosition failed (" + current().path.stem().string() + ")!");
		}
		playtime.add(skipTime);
	}
}

core::Time core::Playlist::currentMusicElapsedTime()
{
	return playtime.getElapsedTime();
}

core::Time core::Playlist::currentMusicDuration()
{
	return current().duration; // IMPORTANT: If Mix_MusicDuration() is used frequently, the music stutters..
}

void core::Playlist::currentMusicSetLoop(bool loop)
{
	// TODO: Does this really work nicely?
	Mix_PlayMusic(music, loop? -1 : 0);
	currentMusicLoop = loop;
	if (Mix_SetMusicPosition(playtime.getElapsedTime().asSeconds()) == -1) {
		log("Mix_SetMusicPosition failed (" + current().path.stem().string() + ")!");
	}
}

void core::Playlist::setLoop(bool loop)
{
	this->loop = loop;
}

bool core::Playlist::isPlaying() const
{
	return Mix_PlayingMusic() && !Mix_PausedMusic(); // SDL2 treats paused music as playing music
}

bool core::Playlist::isPaused() const
{
	return Mix_PausedMusic();
}

bool core::Playlist::isCurrentMusicStopped() const
{
	return !Mix_PlayingMusic(); // Important you can not call !isPlaying() here; SDL2 returns true if music is paused.
}

bool core::Playlist::isStopped() const
{
	return isCurrentMusicStopped() && current_ >= playlist.size();
}

bool core::Playlist::currentMusicGetLoop() const
{
	return currentMusicLoop;
}

bool core::Playlist::getLoop() const
{
	return loop;
}

float core::Playlist::getVolume() const
{
	return volume; // do not return music.getVolume() because of the fading I do
}

core::Playlist::Entry& core::Playlist::current()
{
	assert(!playlist.empty());
	return playlist.at(current_);
}

const core::Playlist::Entry& core::Playlist::current() const
{
	return playlist.at(current_);
}

size_t core::Playlist::currentNumber() const
{
	return current_ + 1;
}

std::vector<fs::path> core::Playlist::getMusicDirs() const
{
	return musicDirs;
}

size_t core::Playlist::size() const
{
	return playlist.size();
}

core::Playlist::Entry& core::Playlist::at(size_t index)
{
	return playlist.at(index);
}