# ConsoleMusicPlayer

This music player uses as interface a console and can run playlists. Used is the SDL2 audio system.

## Installation
This program supports Windows (x86, x64). Just download, open Visual Studio, put SDL2 .dlls to your executable, compile and run. 

## How to use this music player?
When running the program you first have to select a playlist. By default there is a 'all' playlist which plays all music. Music is searched for in specific folders specified in config.dat. If multiple playlists are displayed, then select them with the arrow keys or by typing the displayed number. Afterwards just press Enter and the playlist runs. In this mode you can press [I] to see all available key commands. Enjoy!
#### Create playlists!
If you like, you can add a playlist to data/. In a playlist (.pl) file in each line is the filename (e.g. myMusic.mp3) of a music file (which has to be in an folder specified in config.dat::musicDirs). Note: PlaylistEditor is comming, but for now dirToPlaylist.py helps to somewhat create playlists:

```powershell
PS D:\...\Console_MusicPlayer> python dirToPlaylist.py "C:\Users\MyName\Musik"
```

This will output an <dir-name>.pl file to your current directory which contains all music pieces who are directly inside this directory. 

## What is done next?
- I want to add an PlaylistEditor (add, remove music from a playlist and create new ones and specify directories).
