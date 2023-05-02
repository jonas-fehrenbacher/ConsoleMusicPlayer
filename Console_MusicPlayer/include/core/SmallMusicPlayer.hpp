#pragma once

#include "Playlist.hpp"
#include "Console.hpp"

namespace core
{
	class SmallMusicPlayer
	{
	public:
		enum Replay
		{
			None = 0, // Loop deactivated
			One = 1, // Loop currently playing music
			All = 2, // Loop playlist - all music
			Count = 3
		};

		struct Style
		{
			core::Color title;
			core::Color duration;
			core::Color status;
			core::Color statusOn;
			core::Color statusOff;
			core::Color border;
		};

		core::Text  skipReport;
		Replay      replay;
		core::Text  volumeReport;

		SmallMusicPlayer();

		void init(std::vector<fs::path> musicDirs, int drawSize, int drawPosX, Style style);
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
		Playlist playlist;
		int      drawSize;
		int      drawPosX;
		Style    style;

		
		core::Timer cooldownVolumeReport;
		
		core::Timer cooldownSkipReport;
		bool        drawKeyInfo;
	};
}