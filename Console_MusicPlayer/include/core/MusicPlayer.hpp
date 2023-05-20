#pragma once

#include "Timer.hpp"
#include "DrawableList.hpp"
#include <SDL.h>
#include <SDL_mixer.h>
#include <string>
#include <filesystem>
#include <vector>
namespace fs = std::filesystem;

namespace core
{
	/**
	 * Music player has playlists and plays them.
	 * Usage:
	 * - First you have to call MusicPlayer::init(). This loads all music sets everything up.
	 * - Afterwards you can add playlists with MusicPlayer::addPlaylist(). Note there is a default playlist called ALL_PLAYLIST_NAME which contains all found tracks.
	 *   The playlist file contains in each line a music filename (helloWorld.mp3) which is searched for in all directories specified in 'musicDirPaths'.
	 * - To run a playlist use playPlaylist(name). The playlist name is the filename without its extention - its stem name. Or just use Options::AutoStart
	 *   to automatically start the ALL_PLAYLIST_NAME playlist when initializing.
	 * - By default the ALL_PLAYLIST_NAME playlist is drawn. To draw something else use MusicPlayer::setDrawnPlaylist(). This is not done automatically, because
	 *   a playlist can be played, while the user looks at a different playlist.
	 * - MusicPlayer can even handle events. TODO: Select events and its key bindings.
	 */
	class MusicPlayer
	{
	public:
		/* flags */
		enum Options
		{
			Shuffle   = 1 << 0,
			LoopAll   = 1 << 1,
			LoopOne   = 1 << 2,
			FadeOut   = 1 << 3,
			AutoStart = 1 << 4
		};

		enum class Replay
		{
			None  = 0, // Loop deactivated
			One   = 1, // Loop currently playing music
			All   = 2, // Loop playlist - all tracks
			Count = 3
		};

		struct MusicInfo
		{
			std::filesystem::path path;
			std::string           title;
			std::string           artist;
			std::string           album;
			Time                  duration;
		};

		static const std::string ALL_PLAYLIST_NAME;

		// Deprecated:
		core::Text skipReport;
		core::Text volumeReport;

		void init(std::vector<fs::path> musicDirPaths, DrawableList::Style style, fs::path configFilePath, int options = 0, Time sleepTime = 0ns);
		void terminate();
		/** Does nothing if playlist is already added. */
		void addPlaylist(fs::path playlistFilePath);
		void update();
		void handleEvents();
		void draw();
		void playPlaylist(std::string playlistName, int startTrack = -1);
		void resume();
		void pause();
		void stop();
		/* Shuffle everything and currently playing music is the first entry. */
		void shuffle();
		/** Only resets list order. Music is still played from index 0 to n, so this could cause that some music is played twice or never. */
		void resetShuffle();
		/** call this if console is resized */
		void onConsoleResize();
		/** Useful if you want to draw the playlist, but do not want to scroll. */
		void stopDrawableListEvents();
		void resumeDrawableListEvents();
		void scrollDrawableListToTop();

		/** playlistName can be emptry to display nothing. */
		void setDrawnPlaylist(std::string playlistName = "");
		void setVolume(float volume);
		/* Music may also be paused. */
		const MusicInfo& getPlayingMusicInfo() const;
		const Time getPlayingMusicElapsedTime() const;
		std::string getActivePlaylistName() const;
		int getActivePlaylistSize() const;
		int getActivePlaylistCurrentTrackNumber() const;
		float getVolume() const;
		Replay getReplayStatus() const;
		bool isPlaying() const;
		bool isPaused() const;
		bool isStopped() const;
		bool empty() const;
		bool isShuffled() const;
		bool isTrappedOnTop() const;
	private:
		struct Playlist
		{
			std::string      name;
			std::vector<int> musicIndexList; //< music index from musicInfoList
			DrawableList     drawableList; //< is in the order of the currently playing playlist; Is in 'Playlist', so that unactive playlists can be drawn.
		};

		Mix_Music*                     music; // currently playing music
		std::vector<fs::path>          musicDirPaths;
		std::vector<MusicInfo>         musicInfoList; //< contains all found music files.
		std::vector<Playlist>          playlists;
		Playlist*                      activePlaylist; //< currently active playlist
		Playlist*                      drawnPlaylist; //< playlist which is drawn.
		std::vector<int>               playingOrder; //< specifies the playing order from the "music" in Playlist::musicIndexList; 0..musicIndexList.size()
		int                            playingOrder_currentIndex; // index of current music in playingOrder
		core::Timer                    trackPlaytime;
		core::Time                     sleepTime; //< user can define how long the player should play, when it should put itself to sleep.
		core::Timer                    playtime; //< started with the first track being played 
		bool                           fadeOutEnabled;
		bool                           fadeOutActive;
		float                          volume;
		bool                           isShuffled_;
		Replay                         replayStatus;
		core::Timer                    cooldownSkipReport;
		core::Timer                    cooldownVolumeReport;
		core::DrawableList::InitInfo   drawableList_initInfo;
		fs::path                       configFilePath;

		void addMusic(std::filesystem::path musicFilePath);
		/* return playing music index from Playlist::musicIndexList */
		int getPlaylistPlayingMusicIndex() const;
		/* return playing music index from MusicPlayer::musicInfoList */
		int getPlayingMusicIndex() const;
		void play(bool next);
		void skipTime(Time time);
		void updateListSelection();
	};
}