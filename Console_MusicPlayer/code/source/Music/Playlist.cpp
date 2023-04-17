#include "Music/Playlist.hpp"
#include "Tools/Tool.hpp"
#include <filesystem>
#include <fstream>
#include <random>
#include <iostream>
#include <cassert>
namespace fs = std::filesystem;

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
	fadeOutEnabled(false)
{

}

void core::Playlist::addNewEntry(std::filesystem::path path)
{
	if (!fs::exists(path)) {
		std::ofstream ofs("data/log.txt", std::ios_base::app);
		ofs << "File (" << path.string() << ") not found!\n";
		ofs.close();
		return;
	}

	// TODO: Read artist etc. with SDL2
	std::string title = path.stem().string();
	Entry entry;
	entry.path = path.string();
	entry.title = title;
	entry.artist = "unknown";
	entry.playCount = 1;
	playlist.push_back(entry);
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

void core::Playlist::init(std::filesystem::path playlistPath, int options /*= 0*/)
{
	if (playlistPath.empty() || playlistPath == "all") {
		init();
		return;
	}
	else if (!fs::exists(playlistPath)) {
		std::cout << "Error: Playlist (" << playlistPath << ") not found!\n";
		system("PAUSE");
	}

	name = playlistPath.filename().string();
	std::string   filename;
	std::ifstream ifs(playlistPath, std::ios::in);

	while (std::getline(ifs, filename)) {
		addNewEntry("music/" + filename); // TODO: User can specify multiple folders where music is stored. Find filename in one of them instead of statically using "music/".
	}

	ifs.close();

	_init(options);
}

void core::Playlist::init(int options /*= 0*/)
{
	for (auto& p : fs::directory_iterator("music")) {
		addNewEntry(p.path());
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
		music.openFromFile(current().path.string());
		music.play();
	}
	playtime.restart();
}

void core::Playlist::terminate()
{
	playlist.clear();
	name = "";
	current_ = 0;
	loop = false;
	music.stop();
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
			music.openFromFile(current().path.string());
			music.play();
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
		music.play();
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
			music.setVolume(volume * fadeFactor); // important do not call this->setVolume() here.
		}
		else if (fadeOutActive && remainingPlaytime.asSeconds() > 10.f) {
			fadeOutActive = false;
			setVolume(volume);
		}
	}
}

void core::Playlist::resume()
{
	music.play();
	playtime.resume();
}

void core::Playlist::pause()
{
	music.pause();
	playtime.stop();
}

void core::Playlist::stop()
{
	music.stop();
	playtime.stop();
}

void core::Playlist::setVolume(float volume)
{
	music.setVolume(volume);
	this->volume = volume;
}

void core::Playlist::skipTime(Time skipTime)
{
	float time = playtime.getElapsedTime().asSeconds() + skipTime.asSeconds();
	if (time < 0) {
		music.setPlayingOffset(sf::seconds(0));
		playtime.restart();
	}
	else {
		music.setPlayingOffset(sf::seconds(time));
		playtime.add(skipTime);
	}
}

core::Time core::Playlist::currentMusicElapsedTime()
{
	return playtime.getElapsedTime();
}

core::Time core::Playlist::currentMusicDuration()
{
	return Time(Microseconds(music.getDuration().asMicroseconds()));
}

void core::Playlist::currentMusicSetLoop(bool loop)
{
	music.setLoop(loop);
}

void core::Playlist::setLoop(bool loop)
{
	this->loop = loop;
}

bool core::Playlist::isPlaying() const
{
	return music.getStatus() == sf::Music::Playing;
}

bool core::Playlist::isPaused() const
{
	return music.getStatus() == sf::Music::Paused;
}

bool core::Playlist::isCurrentMusicStopped() const
{
	return music.getStatus() == sf::Music::Stopped;
}

bool core::Playlist::isStopped() const
{
	return isCurrentMusicStopped() && current_ >= playlist.size();
}

bool core::Playlist::currentMusicGetLoop() const
{
	return music.getLoop();
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

size_t core::Playlist::size() const
{
	return playlist.size();
}

core::Playlist::Entry& core::Playlist::at(size_t index)
{
	return playlist.at(index);
}