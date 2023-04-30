#pragma once

#include "Timer.hpp"
#include <SDL.h>
#include <SDL_mixer.h>
#include <string>
#include <filesystem>
#include <vector>
namespace fs = std::filesystem;

namespace core
{
	/* Uses Mix_HookMusicFinished(), so do not overwrite the set callback. */
	class Playlist
	{
	public:
		struct Entry
		{
			std::filesystem::path path;
			std::string           title;
			std::string           artist;
			std::string           album;
			Time                  duration;
			unsigned long         playCount; // How many times in a row should this be played?
			size_t                originSortIndex; // number in which it got initialized (required to reset shuffle)
		};

		/* flags */
		enum Options
		{
			Shuffle   = 1 << 0,
			Loop      = 1 << 1,
			FadeOut   = 1 << 2,
			AutoStart = 1 << 3
		};

		Playlist();
		/* Load playlist from file (must be located in data/). If playlistName is empty, then all music is loaded. */
		void init(std::filesystem::path playlistName, std::vector<fs::path> musicDirs, int options = 0);
		/* Create a playlist with all music there is. */
		void init(std::vector<fs::path> musicDirs, int options = 0);
		void terminate();
		void playNext();
		void playPrevious();
		void update();
		/* Index of music is known if the playlist is not shuffled and if you use the same list as this playlist.
			init(musicDirs) for example creates a list by iterating all directories in order and using 
			fs::recursive_directory_iterator. */
		void start(size_t index = 0);
		void resume();
		void pause();
		void stop();
		/* Shuffle everything and currently playing music is the first entry. */
		void shuffle();
		/** Only resets list order. Music is still played from index 0 to n, so this could cause that some music is played twice or never. */
		void resetShuffle();
		void skipTime(Time time);
		void setVolume(float volume);
		void setLoop(bool loop);
		void setCurrentMusicLoop(bool loop);
		Time getCurrentMusicDuration() const;
		Time getCurrentMusicElapsedTime() const;
		/** Get number of currently playing music; range: 0..size(). This is index + 1 (but index of shuffled playlist). */
		size_t getCurrentMusicNumber() const;
		/** 
		 * Get original index of currently playing music. This is the index with which the music list got initialized. 
		 * If you have for example a ScrollableList with this order, then you can use this function to for example
		 * select the currently playing music.
		 */
		size_t getCurrentMusicIndex() const;
		bool getCurrentMusicLoop() const;
		bool getLoop() const;
		float getVolume() const;
		size_t size() const;
		Entry& at(size_t index);
		const Entry& at(size_t index) const;
		Entry& current();
		const Entry& current() const;
		/* Happens when music finished playing. */
		bool isPlaying() const;
		bool isPaused() const;
		/* Playlist stops automatically after all music is played and loop isn't activated. */
		bool isStopped() const;
		bool isCurrentMusicStopped() const;
		bool isShuffled() const;
	private:
		std::vector<Entry> playlist;
		std::vector<int>   shuffleOrder;
		std::string        name;
		size_t             current_; // index of current playing music
		bool               loop;
		Mix_Music*         music;
		core::Timer        playtime;
		bool               fadeOutEnabled;
		bool               fadeOutActive;
		float              volume;
		bool               currentMusicLoop;
		bool               isShuffled_;

		void addNewEntry(fs::path);
		void play(bool next);
		void _init(int options);
		/* If the user uses an index, then its the original index from the list with which he initialized the playlist. */
		size_t originalToRealIndex(size_t originalIndex) const;
		size_t realToOriginalIndex(size_t realIndex) const;
	};
}