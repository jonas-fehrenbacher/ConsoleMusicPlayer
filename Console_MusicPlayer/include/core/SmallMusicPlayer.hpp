#pragma once

#include "Playlist.hpp"
#include "ColoredStr.hpp"

namespace core
{
	class SmallMusicPlayer
	{
	public:
		SmallMusicPlayer();

		void init(std::vector<fs::path> musicDirs, int drawSize, int drawPosX);
		void terminate();
		void update();
		void handleEvent();
		void draw();

		void play(std::string name);
		void play(int index);
		/* Console position in character count. */
		void setPosX(int posX);
		void setDrawKeyInfo(bool drawKeyInfo);
		/* return -1 if there is no music active. */
		int getCurrentMusicIndex();
		const core::Playlist& getPlaylist() const;
		bool isStopped();
	private:
		enum Replay
		{
			None  = 0, // Loop deactivated
			One   = 1, // Loop currently playing music
			All   = 2, // Loop playlist - all music
			Count = 3
		};

		Playlist playlist;
		int      drawSize;
		int      drawPosX;

		core::ColoredStr volumeReport;
		core::Timer      cooldownVolumeReport;
		core::ColoredStr skipReport;
		core::Timer      cooldownSkipReport;
		bool             drawKeyInfo;
		Replay           replay;
	};
}