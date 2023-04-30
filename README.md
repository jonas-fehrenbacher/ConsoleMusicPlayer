# ConsoleMusicPlayer

This music player uses as interface a console and can run playlists. Used is the SDL2 audio system.

## Installation
This program supports Windows (x86, x64). Just download, open Visual Studio, put SDL2 .dlls to your executable, compile and run. 

## How to use this music player?
After you started the program and all music is loaded, you are in Menu>"All music" where you can scroll (with your mouse wheel) through the list of found music on your system. If you want to play the currently highlighted music just press Enter. On the bottom is a small music player where you see the name of the currently playing music, duration, shuffle status, loop status, playing status, volumn status and options to skip a few seconds forward or jump to the music after or before the current one.
In the menu you have on the top three options: "All music" (selected by default), Playlists and Directories. "Directories" just lists the directories in which it is searched for music. Under Playlists you can choose a playlist which brings one to the playlist player in which one has additional info and options on each music and of course only a the selected amount of music is processed (press 'B' to go back to the menu). By default there is a 'all' playlist which plays all music.
Music is searched for in specific folders specified in config.dat.
Enjoy!
#### Create playlists!
If you like, you can add a playlist to data/. In a playlist (.pl) file in each line is the filename (e.g. myMusic.mp3) of a music file (which has to be in an folder specified in config.dat::musicDirs). Note: PlaylistEditor is comming, but for now dirToPlaylist.py helps to somewhat create playlists:

```powershell
PS D:\...\Console_MusicPlayer> python dirToPlaylist.py "C:\Users\MyName\Musik"
```

This will output an \<dir-name\>.pl file to your current directory which contains all music pieces who are directly inside this directory. 

## How does it look?
### Menu > All Music
<img src="preview-imgs/consoleMusicPlayer_menuPlayingMusic.PNG" alt="consoleMusicPlayer_menuPlayingMusic"/>
### Menu > Playlists
<img src="preview-imgs/consoleMusicPlayer_menuPlaylists.PNG" alt="consoleMusicPlayer_menuPlaylists"/>
### Menu > Directories
<img src="preview-imgs/consoleMusicPlayer_menuDirectories.PNG" alt="consoleMusicPlayer_menuDirectories"/>
### Menu > Playlists > Playlist
<img src="preview-imgs/consoleMusicPlayer_playlist.PNG" alt="consoleMusicPlayer_playlist"/>

## Known issues
- Resizing the console can cause issues (horizontal resizing should be fine, though).

## What is done next?
- I want to add an PlaylistEditor (add, remove music from a playlist and create new ones and specify directories).
- Search bar