#pragma once

#include "Timer/Timer.hpp"
#include <SFML/Audio.hpp>
#include <string>
#include <filesystem>

namespace core
{
	class Playlist
	{
	public:
		struct Entry
		{
			std::filesystem::path path;
			std::string           title;
			std::string           artist;
			unsigned long         playCount; // How many times in a row should this be played?
		};

		/* flags */
		enum Options
		{
			Shuffle = 1 << 0,
			Loop    = 1 << 1,
			FadeOut = 1 << 2
		};

		Playlist();
		/* Load playlist from file (must be located in data/). If playlistName is empty, then all music is loaded. */
		void init(std::filesystem::path playlistName, int options = 0);
		/* Create a playlist with all music there is. */
		void init(int options = 0);
		void terminate();
		void playNext();
		void playPrevious();
		void update();

		void resume();
		void pause();
		void stop();
		void setVolume(float volume);
		void skipTime(Time time);
		Time currentMusicElapsedTime();
		Time currentMusicDuration();
		void currentMusicSetLoop(bool loop);
		void setLoop(bool loop);
		void shuffle();

		/* Happens when music finished playing. */
		bool isCurrentMusicStopped() const;
		bool isPlaying() const;
		bool isPaused() const;
		/* Playlist stops automatically after all music is played and loop isn't activated. */
		bool isStopped() const;
		bool currentMusicGetLoop() const;
		bool getLoop() const;
		float getVolume() const;
		size_t size() const;
		Entry& at(size_t index);
		Entry& current();
		const Entry& current() const;
		/* Get number of currently playing music - maximum is size(). This is index + 1. */
		size_t currentNumber() const;
	private:
		std::vector<Entry> playlist;
		std::string        name;
		size_t             current_; // index of current playing music
		bool               loop;
		sf::Music          music;
		core::Timer        playtime;
		Time               oldElapsedTime;
		bool               fadeOutEnabled;
		bool               fadeOutActive;
		float              volume;

		void addNewEntry(std::filesystem::path);
		void play(bool next);
		void _init(int options);
		void addAll(std::filesystem::path dir);
	};
}