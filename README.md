# ConsoleMusicPlayer

This music player uses as interface a console and can run playlists. Used is the SFML audio system.

## Installation
This program supports only Win32 properly. Just download, open Visual Studio, switch architecture to Win32 and it should compile.

## How to use this music player?
When running the program you first have to select a playlist. By default there is a 'all' playlist which plays all
music. Music is searched for in specific folders specified in config.dat. If you like, you can add a playlist to data/. In a playlist (.pl) file in each line is the filename (e.g. myMusic.wav) of a music file (which has to be in an folder specified in config.dat::musicDirs). Note: PlaylistEditor is comming. If multiple playlists are displayed, then select them with the arrow keys or by typing the displayed number. Afterwards just press Enter and the playlist runs. In this mode you can press [I] to see all available key commands. Enjoy!

## What is done next?
- I want to add an PlaylistEditor (add, remove music from a playlist and create new ones and specify directories).
- Use SDL2 to read ID3 data and to support more audio formats.
