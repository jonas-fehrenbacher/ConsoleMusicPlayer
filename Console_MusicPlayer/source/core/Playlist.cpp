#include "core/Playlist.hpp"
#include "core/SmallTools.hpp"
#include <filesystem>
#include <fstream>
#include <random>
#include <iostream>
#include <cassert>

intern core::Playlist* currActivePlaylist = nullptr; // this is for onMusicFinished().

// Called when current playing music finishes or Mix_HaltMusic() is explicitly called.
intern void onMusicFinished()
{
	if (!currActivePlaylist->getCurrentMusicLoop()) {
		currActivePlaylist->playNext();
	}
}

core::Playlist::Playlist() :
	playlist({}),
	name(""),
	current_(0), // must be 0 (see shuffle)
	loop(false),
	music(),
	playtime(),
	fadeOutActive(false),
	volume(100), // default
	fadeOutEnabled(false),
	currentMusicLoop(false),
	isShuffled_(false)
{
	// Play automatically the next music:
	Mix_HookMusicFinished(onMusicFinished);
}

void core::Playlist::init(std::filesystem::path playlistPath, std::vector<fs::path> musicDirs, int options /*= 0*/)
{
	if (playlistPath.empty() || playlistPath == "data/all.pl") {
		init(musicDirs, options);
		return;
	}
	else if (!fs::exists(playlistPath)) {
		std::cout << "Error: Playlist (" << playlistPath << ") not found!\n";
		system("PAUSE");
	}

	name = playlistPath.filename().string();
	std::wstring   filename;
	std::wifstream ifs(playlistPath, std::ios::in);

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

void core::Playlist::init(std::vector<fs::path> musicDirs, int options /*= 0*/)
{
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
	if (hasFlag(AutoStart, options)) {
		start(realToOriginalIndex(0));
	}
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
	entry.title = strcmp(Mix_GetMusicTitle(music), "") == 0 ? filenameStem : Mix_GetMusicTitle(music); // SDL2 does not return filename as mentioned, so I do it manually.
	entry.artist = strcmp(Mix_GetMusicArtistTag(music), "") == 0 ? "unknown" : Mix_GetMusicTitle(music);
	entry.album = strcmp(Mix_GetMusicAlbumTag(music), "") == 0 ? "unknown" : Mix_GetMusicTitle(music);
	entry.duration = Time(Seconds((int)Mix_MusicDuration(music))); // IMPORTANT!: needs to be set once. IF this is called frequently, then the played music stutters!!!
	entry.playCount = 1;
	entry.originSortIndex = playlist.size();
	playlist.push_back(entry);
	Mix_FreeMusic(music);
}

void core::Playlist::terminate()
{
	playlist.clear();
	name = "";
	current_ = 0;
	loop = false;
	Mix_HaltMusic();
	Mix_FreeMusic(music);
	fadeOutEnabled = false;
	fadeOutActive = false;
	volume = 100;
	isShuffled_ = false;
}

void core::Playlist::playNext()
{
	play(true);
}

void core::Playlist::playPrevious()
{
	play(false);
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
			else if (current_ > 0) {
				--current_;
			}
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

void core::Playlist::update()
{
	if (playlist.empty()) {
		return;
	}

	// Loop:
	if (getCurrentMusicLoop() && playtime.getElapsedTime() >= getCurrentMusicDuration()) {
		// ..current music restarts automatically if loop=-1, but playtime needs to be restarted.
		playtime.restart();
	}

	// Fade out:
	if (fadeOutEnabled) {
		Time remainingPlaytime = getCurrentMusicDuration() - playtime.getElapsedTime();
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

void core::Playlist::start(size_t musicID /*= 0*/)
{
	current_ = originalToRealIndex(musicID);
	currActivePlaylist = this;
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

void core::Playlist::shuffle()
{
	static std::random_device rd;
	static auto rng = std::mt19937(rd());
	if (isCurrentMusicStopped()) {
		// No music is playing or paused, so shuffle everything.
		std::shuffle(std::begin(playlist), std::end(playlist), rng);
	}
	else {
		// Shuffling only the remaining music is a bad idea, beacause if the playlist loops, then if the shuffle happend
		// at the last music, the playlist would not be shuffled at all. For instance the Samsung music player replays 
		// everything after an shuffle.
		Entry currEntry = playlist[current_];
		playlist.erase(playlist.begin() + current_);
		playlist.insert(playlist.begin(), currEntry);
		std::shuffle(std::begin(playlist) + 1, std::end(playlist), rng);
		current_ = 0;
	}
	isShuffled_ = true;
}

void core::Playlist::resetShuffle()
{
	int currentoriginIndex = current().originSortIndex;
	std::sort(playlist.begin(), playlist.end(), [](const Entry& a, const Entry& b) {
		return a.originSortIndex < b.originSortIndex;
	});
	current_ = originalToRealIndex(currentoriginIndex); // Stay where you are (when displaying a list in original order).
	isShuffled_ = false;
}

size_t core::Playlist::originalToRealIndex(size_t originalIndex) const
{
	for (int i = 0; i < playlist.size(); ++i) {
		if (playlist[i].originSortIndex == originalIndex) {
			return i;
		}
	}

	return 0; // reached if playlist.empty()
}

size_t core::Playlist::realToOriginalIndex(size_t realIndex) const
{
	if (playlist.empty()) {
		return 0; // playlist[] may not be called!
	}
	return playlist.at(realIndex).originSortIndex;
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

void core::Playlist::setVolume(float volume)
{
	Mix_VolumeMusic(volume);
	this->volume = volume;
}

void core::Playlist::setLoop(bool loop)
{
	this->loop = loop;
}

void core::Playlist::setCurrentMusicLoop(bool loop)
{
	// TODO: Does this really work nicely?
	Mix_PlayMusic(music, loop ? -1 : 0);
	currentMusicLoop = loop;
	if (Mix_SetMusicPosition(playtime.getElapsedTime().asSeconds()) == -1) {
		log("Mix_SetMusicPosition failed (" + current().path.stem().string() + ")!");
	}
}

core::Time core::Playlist::getCurrentMusicDuration() const
{
	return current().duration; // IMPORTANT: If Mix_MusicDuration() is used frequently, the music stutters..
}

core::Time core::Playlist::getCurrentMusicElapsedTime() const
{
	return playtime.getElapsedTime();
}

size_t core::Playlist::getCurrentMusicNumber() const
{
	return current_ + 1;
}

size_t core::Playlist::getCurrentMusicIndex() const
{
	return current().originSortIndex;
}

bool core::Playlist::getCurrentMusicLoop() const
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

size_t core::Playlist::size() const
{
	return playlist.size();
}

core::Playlist::Entry& core::Playlist::at(size_t index)
{
	return playlist.at(originalToRealIndex(index));
}

const core::Playlist::Entry& core::Playlist::at(size_t index) const
{
	return playlist.at(originalToRealIndex(index));
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

bool core::Playlist::isPlaying() const
{
	return Mix_PlayingMusic() && !Mix_PausedMusic(); // SDL2 treats paused music as playing music
}

bool core::Playlist::isPaused() const
{
	return Mix_PausedMusic();
}

bool core::Playlist::isStopped() const
{
	return isCurrentMusicStopped() && current_ >= playlist.size();
}

bool core::Playlist::isCurrentMusicStopped() const
{
	return !Mix_PlayingMusic(); // Important you can not call !isPlaying() here; SDL2 returns true if music is paused.
}

bool core::Playlist::isShuffled() const
{
	return isShuffled_;
}