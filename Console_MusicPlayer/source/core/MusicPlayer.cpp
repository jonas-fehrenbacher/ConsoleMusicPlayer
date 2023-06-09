﻿#include "core/MusicPlayer.hpp"
#include "core/SmallTools.hpp"
#include "core/Profiler.hpp"
#include "core/InputDevice.hpp"
#include "App.hpp"
#include <filesystem>
#include <fstream>
#include <random>
#include <iostream>
#include <cassert>
#include <Windows.h>

const std::string core::MusicPlayer::ALL_PLAYLIST_NAME = "__all8756234875.pl"; //< should be a name nobody chooses for his playlists.

void core::MusicPlayer::init(App* app, int options /*= 0*/, Time sleepTime /*= 0ns*/)
{
	///////////////////////////////////////////////////////////////////////////////
	// Reset all values
	///////////////////////////////////////////////////////////////////////////////
	this->app = app;
	music = nullptr;
	musicInfoList.clear();
	playlists.clear();
	activePlaylist = nullptr;
	drawnPlaylist = nullptr;
	playingOrder.clear();
	playingOrder_currentIndex = -1;
	trackPlaytime;
	this->sleepTime = sleepTime;
	playtime;
	fadeOutEnabled = false;
	volume = 100;
	isShuffled_ = false;
	replayStatus = Replay::None;
	cooldownSkipReport;
	cooldownVolumeReport;
	drawableList_initInfo = {};

	///////////////////////////////////////////////////////////////////////////////
	// Set drawable lists layout
	///////////////////////////////////////////////////////////////////////////////
	std::vector<core::DrawableList::Column> listInitInfo_columnLayout;
	{
		core::DrawableList::Column column;
		column.length            = core::DrawableList::Column::LARGEST_ITEM;
		column.color             = core::Color::Gray;
		column.alignRight        = true;
		column.isVisible         = true;
		column.hasEmptySpace     = false;
		column.isLengthInPercent = false;
		listInitInfo_columnLayout.push_back(column);
		column.length            = 0;
		column.color             = core::Color::Bright_White;
		column.alignRight        = false;
		column.isVisible         = true;
		column.hasEmptySpace     = true;
		column.isLengthInPercent = false;
		listInitInfo_columnLayout.push_back(column);
		column.length            = core::DrawableList::Column::LARGEST_ITEM;
		column.color             = core::Color::Aqua;
		column.alignRight        = true;
		column.isVisible         = true;
		column.hasEmptySpace     = false;
		column.isLengthInPercent = false;
		listInitInfo_columnLayout.push_back(column);
	}
	core::DrawableList::Options listInitInfo_options = (core::DrawableList::Options)(
		(int)core::DrawableList::SelectionMode |
		//(int)core::DrawableList::ArrowInput | // I need this keys for changing the track
		(int)core::DrawableList::DrawFullX // When this is set then use DrawableList::getPosX()
	);
	drawableList_initInfo.options             = listInitInfo_options;
	drawableList_initInfo.style               = app->style.drawableList;
	drawableList_initInfo.name                = "tracks";
	drawableList_initInfo.columnLayout        = listInitInfo_columnLayout;
	drawableList_initInfo.spaceBetweenColumns = 3;
	drawableList_initInfo.sizeInside          = { 60, 20 };
	drawableList_initInfo.hover               = 0;

	///////////////////////////////////////////////////////////////////////////////
	// Load music
	///////////////////////////////////////////////////////////////////////////////
	for (auto& musicDirPath : app->musicDirs) {
		if (fs::exists(musicDirPath)) {
			for (auto& it : fs::recursive_directory_iterator(musicDirPath)) {
				addMusic(it.path().wstring()); // IMPORTANT use here u8string to support Dvořák, but for something like 音楽 you need wstring.
			}
		}
		else {
			log("Error: Music directory '" + musicDirPath.string() + "' could not be found!\n");
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// Set default playlist
	///////////////////////////////////////////////////////////////////////////////
	// Init playlist:
	Playlist allPlaylist; // a playlist which plays all available music
	allPlaylist.name = ALL_PLAYLIST_NAME;
	allPlaylist.musicIndexList.reserve(musicInfoList.size());
	for (int i = 0; i < musicInfoList.size(); ++i)
		allPlaylist.musicIndexList.push_back(i);
	// Set drawable list:
	allPlaylist.drawableList.init(drawableList_initInfo);
	for (int musicIndex : allPlaylist.musicIndexList) {
		MusicInfo& musicInfo = musicInfoList[musicIndex];
		allPlaylist.drawableList.push_back({ musicInfo.title, core::getTimeStr(musicInfo.duration) });
	}
	// Set duration:
	allPlaylist.duration = 0s;
	for (int musicIndex : allPlaylist.musicIndexList) {
		MusicInfo& musicInfo = musicInfoList[musicIndex];
		allPlaylist.duration += musicInfo.duration;
	}
	allPlaylist.oldTracksPlaytime = 0s;
	playlists.push_back(allPlaylist);
	drawnPlaylist = &playlists.back();

	///////////////////////////////////////////////////////////////////////////////
	// Options
	///////////////////////////////////////////////////////////////////////////////
	if (hasFlag(Shuffle, options)) {
		isShuffled_ = true; // shuffle() will be called in playPlaylist()
	}
	if (hasFlag(LoopAll, options)) {
		replayStatus = Replay::All;
	}
	if (hasFlag(LoopOne, options)) {
		replayStatus = Replay::One;
	}
	if (hasFlag(FadeOut, options)) {
		fadeOutEnabled = true;
	}
	if (hasFlag(AutoStart, options)) {
		playPlaylist(ALL_PLAYLIST_NAME);
	}
}

void core::MusicPlayer::addMusic(std::filesystem::path musicFilePath)
{
	if (!fs::exists(musicFilePath)) {
		log("File (" + musicFilePath.string() + ") not found!\n");
		return;
	}

	// Is music file?:
	if (!isSupportedAudioFile(musicFilePath)) {
		return;
	}

	// Open music to get its metadata:
	Mix_Music* music = Mix_LoadMUS(musicFilePath.make_preferred().u8string().c_str());
	if (!music) {
		log("Failed to load music! SDL_mixer Error: " + std::string(Mix_GetError()) + "\n");
		//__debugbreak();
		return;
	}

	std::string filenameStem = musicFilePath.stem().u8string();
	MusicInfo musicInfo = {};
	musicInfo.path      = musicFilePath.make_preferred().wstring();
	musicInfo.title     = strcmp(Mix_GetMusicTitle(music), "") == 0 ? filenameStem : Mix_GetMusicTitle(music); // SDL2 does not return filename as mentioned, so I do it manually.
	musicInfo.artist    = strcmp(Mix_GetMusicArtistTag(music), "") == 0 ? "unknown" : Mix_GetMusicTitle(music);
	musicInfo.album     = strcmp(Mix_GetMusicAlbumTag(music), "") == 0 ? "unknown" : Mix_GetMusicTitle(music);
	musicInfo.duration  = Time(Seconds((int)Mix_MusicDuration(music))); // IMPORTANT!: needs to be set once. IF this is called frequently, then the played music stutters!!!
	musicInfoList.push_back(musicInfo);
	Mix_FreeMusic(music);
}

void core::MusicPlayer::addPlaylist(fs::path playlistFilePath)
{
	PROFILE_FUNC;

	if (!fs::exists(playlistFilePath)) {
		std::cout << "Error: Playlist (" << playlistFilePath << ") not found!\n";
		system("PAUSE");
	}

	for (Playlist& playlist : playlists) {
		if (playlist.name == playlistFilePath.filename().string()) {
			// ..playlist is already set
			return;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// Load music file names
	///////////////////////////////////////////////////////////////////////////////
	std::vector<std::wstring> musicFilenames;
	{
		std::wifstream ifs(playlistFilePath, std::ios::in);
		std::wstring nextFilename;
		while (std::getline(ifs, nextFilename)) {
			musicFilenames.push_back(nextFilename); // Playlist should only hold filenames, so that music files can be moved without editing the playlist file.
		}
		ifs.close();
	}

	///////////////////////////////////////////////////////////////////////////////
	// Setup playlist
	///////////////////////////////////////////////////////////////////////////////
	// Store old playlist:
	// This is neccessary, because if std::vector playlists allocates new memory, then all pointers are invalid.
	std::string activePlaylistName = activePlaylist ? activePlaylist->name : "";
	std::string drawnPlaylistName = drawnPlaylist ? drawnPlaylist->name : "";
	// Add new playlist:
	Playlist newPlaylist;
	newPlaylist.name = playlistFilePath.filename().string();
	newPlaylist.musicIndexList.reserve(musicFilenames.size());
	for (int i = 0; i < musicInfoList.size(); ++i) {
		if (std::find(musicFilenames.begin(), musicFilenames.end(), musicInfoList[i].path.filename().wstring()) != musicFilenames.end()) {
			// ..playlist requires this music
			newPlaylist.musicIndexList.push_back(i);
		}
	}
	// Set drawable list:
	newPlaylist.drawableList.init(drawableList_initInfo);
	for (int musicIndex : newPlaylist.musicIndexList) {
		MusicInfo& musicInfo = musicInfoList[musicIndex];
		newPlaylist.drawableList.push_back({ musicInfo.title, core::getTimeStr(musicInfo.duration) });
	}
	// Set duration:
	newPlaylist.duration = 0s;
	for (int musicIndex : newPlaylist.musicIndexList) {
		MusicInfo& musicInfo = musicInfoList[musicIndex];
		newPlaylist.duration += musicInfo.duration;
	}
	newPlaylist.oldTracksPlaytime = 0s;
	playlists.push_back(newPlaylist);
	// Restore pointers:
	for (Playlist& playlist : playlists) {
		if (playlist.name == activePlaylistName) {
			activePlaylist = &playlist;
		}
		if (playlist.name == drawnPlaylistName) {
			drawnPlaylist = &playlist;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// Log missing files
	///////////////////////////////////////////////////////////////////////////////
	for (std::wstring& playlistFileEntry : musicFilenames) {
		bool found = false;
		for (int playlistEntry : newPlaylist.musicIndexList) {
			if (musicInfoList[playlistEntry].path.filename().wstring() == playlistFileEntry) {
				found = true;
				break;
			}
		}
		if (!found)
			log("Warning: Playlist (" + newPlaylist.name + ") could not find '" + toStr(playlistFileEntry) + "'!");
	}
}

void core::MusicPlayer::terminate()
{
	stop();
	musicInfoList.clear();
	playlists.clear();
	drawnPlaylist = nullptr;
	sleepTime = 0s;
	fadeOutEnabled = false;
	volume = 100;
	isShuffled_ = false;
	replayStatus = Replay::None;
	drawableList_initInfo = {};
}

void core::MusicPlayer::update()
{
	if (empty()) {
		return;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Play next
	///////////////////////////////////////////////////////////////////////////////
	if (activePlaylist && isStopped()) {
		play(true);
		updateListSelection();
	}

	///////////////////////////////////////////////////////////////////////////////
	// Fade out
	///////////////////////////////////////////////////////////////////////////////
	if (activePlaylist && fadeOutEnabled) {
		Time remainingPlaytime = getPlayingMusicInfo().duration - trackPlaytime.getElapsedTime();
		Time fadeOutTime = 10s;
		float minFadeOutFactor = 0.2; // Otherwise I get error messages.
		if (remainingPlaytime <= fadeOutTime) {
			fadeOutActive = true;

			float fadeFactor = remainingPlaytime.asSeconds() / fadeOutTime.asSeconds(); // value from 0..1 (8sec / 10sec = 0.8)
			fadeFactor = fadeFactor < minFadeOutFactor ? minFadeOutFactor : fadeFactor;
			Mix_VolumeMusic(volume * fadeFactor); // important do not call this->setVolume() here.
		}
		else if (fadeOutActive && remainingPlaytime > 10s) {
			fadeOutActive = false;
			setVolume(volume);
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// Update list border color
	///////////////////////////////////////////////////////////////////////////////
	if (drawnPlaylist && isStopped()) {
		drawnPlaylist->drawableList.style.border = core::Color::Light_Red;
	}
	else if (drawnPlaylist && isPaused()) {
		drawnPlaylist->drawableList.style.border = core::Color::Light_Aqua;
	}
	else if (drawnPlaylist && isPlaying()) {
		drawnPlaylist->drawableList.style.border = core::Color::Light_Green;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Sync auto play with music list
	///////////////////////////////////////////////////////////////////////////////
	if (activePlaylist && isPlaying()) {
		activePlaylist->drawableList.select(getPlaylistPlayingMusicIndex());
	}

	///////////////////////////////////////////////////////////////////////////////
	// Update drawn list
	///////////////////////////////////////////////////////////////////////////////
	if (drawnPlaylist) {
		drawnPlaylist->drawableList.update();
	}

	///////////////////////////////////////////////////////////////////////////////
	// Handle timers
	///////////////////////////////////////////////////////////////////////////////
	// Loop one:
	if (activePlaylist && replayStatus == Replay::One && trackPlaytime.getElapsedTime() >= getPlayingMusicInfo().duration) {
		// ..current music restarts automatically if loop=-1, but trackPlaytime needs to be restarted.
		trackPlaytime.restart(); 
	}
	// Sleep:
	if (sleepTime > 0s && playtime.getElapsedTime() >= sleepTime) {
		stop();
	}

	///////////////////////////////////////////////////////////////////////////////
	// Update draw info
	///////////////////////////////////////////////////////////////////////////////
	//volume report:
	if (cooldownVolumeReport.getElapsedTime().asSeconds() > 0.5f && volumeReport.text != "")
	{
		volumeReport.text = "";
		cooldownVolumeReport.restart();
	}
	//skip report:
	if (cooldownSkipReport.getElapsedTime().asSeconds() > 0.5f && skipReport.text != "")
	{
		skipReport.text = "";
		cooldownSkipReport.restart();
	}
}

void core::MusicPlayer::updateListSelection()
{
	if (activePlaylist)
	{
		// ..a track is playing - select it everywhere
		const std::string playingMusicTitle = getPlayingMusicInfo().title;
		for (Playlist& playlist : playlists) {
			for (size_t i = 0; i < playlist.drawableList.get().size(); ++i) {
				if (playlist.drawableList.get()[i][0] == playingMusicTitle) {
					playlist.drawableList.select(i);
					break;
				}
			}
		}
		
	}
	else
	{
		// ..no playlist is selected / nothing is playing - remove all selections
		for (Playlist& playlist : playlists) {
			playlist.drawableList.select(DrawableList::NOINDEX);
		}
	}
}

void core::MusicPlayer::handleEvents()
{
	// Enter key (Select item):
	if (drawnPlaylist && drawnPlaylist->drawableList.hasFocus() && core::inputDevice::isKeyPressed(app->keymap.get(Keymap::Action::Select).key))
	{
		drawnPlaylist->drawableList.selectHoveredItem();
		int playlistMusicIndex = (int)drawnPlaylist->drawableList.getSelectedIndex();
		if (activePlaylist == drawnPlaylist) {
			playingOrder_currentIndex = playlistMusicIndex - 1; // -1 because play(true) will increase index by 1
			play(true); // play next
			updateListSelection();
		}
		else {
			playPlaylist(drawnPlaylist->name, playlistMusicIndex); // calls updateListSelection();
		}
	}

	// Up / Down key:
	if (!isStopped()) {
		if (inputDevice::isKeyPressed(app->keymap.get(Keymap::Action::NextTrack).key)) {
			play(true); // next
		}
		else if (inputDevice::isKeyPressed(app->keymap.get(Keymap::Action::PrevTrack).key)) {
			play(false); // previous
		}
	}

	// R-Key
	if (inputDevice::isKeyPressed(app->keymap.get(Keymap::Action::Shuffle).key)) {
		if (isShuffled()) resetShuffle();
		else shuffle();

		// Update config:
		std::map<std::wstring, std::wstring> config = core::getConfig(app->configFilePath);
		config[L"isPlaylistShuffled"] = isShuffled() ? L"true" : L"false";
		core::setConfig(app->configFilePath, config);
	}

	// L-Key:
	if (inputDevice::isKeyPressed(app->keymap.get(Keymap::Action::Repeat).key)) { // Loop key
		// ..select one of 3 modi
		replayStatus = static_cast<Replay>((int)replayStatus + 1);
		replayStatus = static_cast<Replay>((int)replayStatus % (int)Replay::Count); // wrap around

		// Update config:
		std::map<std::wstring, std::wstring> config = core::getConfig(app->configFilePath);
		config[L"playlistLoop"] = replayStatus == Replay::None ? L"none" : (replayStatus == Replay::One ? L"one" : L"all");
		core::setConfig(app->configFilePath, config);
	}

	// Left-Key:
	core::Time skip_time = 10s;
	std::string skipUnit = "s";
	if (!isStopped() && inputDevice::isKeyPressed(app->keymap.get(Keymap::Action::TrackSkipBackward).key))
	{
		if (getPlayingMusicElapsedTime() < skip_time)
		{
			std::string skippedTimeText = std::to_string(getPlayingMusicElapsedTime().asSeconds());
			int decimalPos = skippedTimeText.find(".");
			decimalPos = decimalPos == -1 ? skippedTimeText.find(",") : decimalPos;
			decimalPos = decimalPos == -1 ? 0 : decimalPos;
			skipReport.text = " -" + skippedTimeText.substr(0, decimalPos + 3) + skipUnit;
			skipReport.isPositive = false;
			if (replayStatus == Replay::One) skipTime(core::Time(core::Seconds((long)skip_time.asSeconds() * -1))); // will be 0
			else play(false);
		}
		else
		{
			skipReport.text = " -" + std::to_string((int)skip_time.asSeconds()) + skipUnit;
			skipReport.isPositive = false;
			skipTime(core::Time(core::Seconds((long)skip_time.asSeconds() * -1)));
		}

		cooldownSkipReport.restart();
	}

	// Right-Key:
	if (!isStopped() && inputDevice::isKeyPressed(app->keymap.get(Keymap::Action::TrackSkipForward).key))
	{
		if (getPlayingMusicElapsedTime().asSeconds() > getPlayingMusicInfo().duration.asSeconds() - skip_time.asSeconds())
		{
			// ..there are no x sec remaining
			core::Time skippedTime = getPlayingMusicInfo().duration - getPlayingMusicElapsedTime();
			std::string skippedTimeText = std::to_string(skippedTime.asSeconds());
			skipReport.text = " +" + skippedTimeText.substr(0, skippedTimeText.find(".") + 3) + skipUnit;
			skipReport.isPositive = true;
			if (replayStatus == Replay::One) skipTime(skippedTime); // Set music to the end, so it loops immediately.
			else play(true);
		}
		else
		{
			skipReport.text = " +" + std::to_string((int)skip_time.asSeconds()) + skipUnit;
			skipReport.isPositive = true;
			skipTime(skip_time);
		}

		cooldownSkipReport.restart();
	}

	// +-Key:
	if (inputDevice::isKeyPressed(app->keymap.get(Keymap::Action::IncreaseVolume).key))
	{
		if (getVolume() <= 95)
		{
			volumeReport.text = "+5%";
			volumeReport.isPositive = true;
			setVolume(getVolume() + 5);
		}
		else
		{
			volumeReport.text = "+" + std::to_string(100 - static_cast<unsigned short>(getVolume())) + "%";
			volumeReport.isPositive = true;
			setVolume(100);
		}

		cooldownVolumeReport.restart();
	}

	// --Key:
	if (inputDevice::isKeyPressed(app->keymap.get(Keymap::Action::DecreaseVolume).key))
	{
		if (getVolume() >= 5)
		{
			volumeReport.text = "-5%";
			volumeReport.isPositive = false;
			setVolume(getVolume() - 5);
		}
		else
		{
			volumeReport.text = "-" + std::to_string(static_cast<unsigned short>(getVolume())) + "%";
			volumeReport.isPositive = false;
			setVolume(0);
		}

		cooldownVolumeReport.restart();
	}

	// P-Key:
	if (!isStopped() && inputDevice::isKeyPressed(app->keymap.get(Keymap::Action::PlayPause).key))
	{
		if (isPlaying())
		{
			pause();
		}
		else if (isPaused())
		{
			resume();
		}
	}

	if (drawnPlaylist) {
		drawnPlaylist->drawableList.handleEvent();
	}
}

void core::MusicPlayer::skipTime(Time skipTime)
{
	Time time = trackPlaytime.getElapsedTime() + skipTime;
	if (time < 0s) {
		if (Mix_SetMusicPosition(0) == -1) {
			log("Mix_SetMusicPosition failed (" + getPlayingMusicInfo().path.stem().string() + ")!");
		}
		trackPlaytime.restart();
	}
	else {
		if (Mix_SetMusicPosition(time.asSeconds()) == -1) {
			log("Mix_SetMusicPosition failed (" + getPlayingMusicInfo().path.stem().string() + ")!");
		}
		trackPlaytime.add(skipTime);
	}
}

void core::MusicPlayer::draw()
{
	if (drawnPlaylist) {
		drawnPlaylist->drawableList.draw();
	}
}

void core::MusicPlayer::playPlaylist(std::string playlistName, int startTrack /*= -1*/)
{
	if (empty()) {
		return;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Stop current playlist
	///////////////////////////////////////////////////////////////////////////////
	stop();

	///////////////////////////////////////////////////////////////////////////////
	// Select playlist
	///////////////////////////////////////////////////////////////////////////////
	for (Playlist& playlist : playlists) {
		if (playlist.name == playlistName)
			activePlaylist = &playlist;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Set playlist order
	///////////////////////////////////////////////////////////////////////////////
	playingOrder.clear();
	for (int i = 0; i < activePlaylist->musicIndexList.size(); ++i) {
		playingOrder.push_back(i);
	}
	playingOrder_currentIndex = startTrack; // required for shuffle [and play()]
	if (isShuffled_) {
		shuffle();
	}

	///////////////////////////////////////////////////////////////////////////////
	// Start playing
	///////////////////////////////////////////////////////////////////////////////
	playingOrder_currentIndex += playingOrder_currentIndex == -1 ? 0 : -1; // to play selected track - reduce this before playing next track.
	play(true);
	playtime.restart();

	///////////////////////////////////////////////////////////////////////////////
	// Update list selection
	///////////////////////////////////////////////////////////////////////////////
	updateListSelection();
}

void core::MusicPlayer::play(bool next)
{
	if (empty()) {
		return;
	}

	if (music != nullptr && replayStatus == Replay::One) {
		// ..replay same track
		Mix_PlayMusic(music, 0);
		trackPlaytime.restart();
		return;
	}
	
	// play next or previous track:
	playingOrder_currentIndex += next ? 1 : -1;

	// Stop playlist:
	if (playingOrder_currentIndex >= playingOrder.size() && replayStatus == Replay::None) 
	{
		// ..playlist end reached
		trackPlaytime.restart();
		trackPlaytime.pause();
		playtime.pause();
		activePlaylist = nullptr;
		playingOrder_currentIndex = 0;
		playingOrder.clear();
		if (music) {
			Mix_HaltMusic();
			Mix_FreeMusic(music);
			music = nullptr;
		}
		return;
	}

	// Loop Playlist:
	if (replayStatus == Replay::All) 
	{
		if (playingOrder_currentIndex >= playingOrder.size()) {
			playingOrder_currentIndex = 0;
		}
		if (playingOrder_currentIndex < 0) {
			playingOrder_currentIndex = playingOrder.size() - 1;
		}
	}

	// Play music:
	if (music) {
		Mix_HaltMusic(); // required if Mix_FadeOutMusic() is used, otherwise no new music can be played till fade is finished.
		Mix_FreeMusic(music);
		music = nullptr;
	}
	music = Mix_LoadMUS(getPlayingMusicInfo().path.u8string().c_str()); // 'getPlayingMusicInfo()' depends on 'playingOrder_currentIndex'
	if (!music) {
		log("Failed to load music! SDL_mixer Error: " + std::string(Mix_GetError()) + "\n");
		__debugbreak();
	}
	Mix_PlayMusic(music, 0);
	// This would start the fade out immediately and there aren't many options, so I do it manually in update().
	//if (fadeOutEnabled) {
	//	Mix_FadeOutMusic(10000);
	//}
	trackPlaytime.restart();

	// Update duration:
	activePlaylist->oldTracksPlaytime = 0s;
	for (int i = 0; i < playingOrder_currentIndex; ++i) { // current music may not be calculated
		activePlaylist->oldTracksPlaytime += musicInfoList[activePlaylist->musicIndexList[playingOrder[i]]].duration;
	}
}

void core::MusicPlayer::resume()
{
	Mix_ResumeMusic();
	trackPlaytime.resume();
}

void core::MusicPlayer::pause()
{
	Mix_PauseMusic();
	trackPlaytime.pause();
}

void core::MusicPlayer::stop()
{
	trackPlaytime.restart();
	trackPlaytime.stop();
	playtime.stop();
	activePlaylist = nullptr;
	playingOrder_currentIndex = -1;
	playingOrder.clear();
	if (music) {
		Mix_HaltMusic();
		Mix_FreeMusic(music);
		music = nullptr;
	}
}

void core::MusicPlayer::shuffle()
{
	static std::random_device rd;
	static auto rng = std::mt19937(rd());

	isShuffled_ = true;

	if (playingOrder.empty()) {
		return;
	}

	assert(activePlaylist && (playingOrder_currentIndex >= -1 || playingOrder_currentIndex < playingOrder.size()));

	if (playingOrder_currentIndex == -1) {
		// No music is playing or paused, so shuffle everything.
		std::shuffle(std::begin(playingOrder), std::end(playingOrder), rng);
	}
	else {
		// Shuffling only the remaining music is a bad idea, beacause if the playlist loops, then if the shuffle happend
		// at the last music, the playlist would not be shuffled at all. For instance the Samsung music player replays 
		// everything after an shuffle.
		int playlist_currentMusicIndex = playingOrder.at(playingOrder_currentIndex);
		playingOrder.erase(playingOrder.begin() + playingOrder_currentIndex);
		playingOrder.insert(playingOrder.begin(), playlist_currentMusicIndex);
		std::shuffle(std::begin(playingOrder) + 1, std::end(playingOrder), rng);
		playingOrder_currentIndex = 0;
	}
}

void core::MusicPlayer::resetShuffle()
{
	if (!playingOrder.empty())
	{
		int playlist_currentMusicIndex = playingOrder.at(playingOrder_currentIndex);
		std::sort(playingOrder.begin(), playingOrder.end(), [](const int& a, const int& b) {
			return a < b;
			});
		for (int i = 0; i < playingOrder.size(); ++i) {
			if (playlist_currentMusicIndex == playingOrder[i]) {
				playingOrder_currentIndex = i;
				break;
			}
		}
	}
	isShuffled_ = false;
}

void core::MusicPlayer::onConsoleResize()
{
	for (Playlist& playlist : playlists) {
		playlist.drawableList.onConsoleResize();
	}
}

void core::MusicPlayer::stopDrawableListEvents()
{
	for (Playlist& playlist : playlists) {
		playlist.drawableList.loseFocus();
	}
}

void core::MusicPlayer::resumeDrawableListEvents()
{
	for (Playlist& playlist : playlists) {
		playlist.drawableList.gainFocus();
	}
}

void core::MusicPlayer::scrollDrawableListToTop()
{
	for (Playlist& playlist : playlists) {
		playlist.drawableList.scrollToTop();
	}
}

void core::MusicPlayer::setDrawnPlaylist(std::string playlistName /*= ""*/)
{
	if (playlistName.empty()) {
		drawnPlaylist = nullptr;
	}
	else if (!playlistName.empty()) {
		bool found = false;
		for (Playlist& playlist : playlists) {
			if (playlist.name == playlistName) {
				drawnPlaylist = &playlist;
				found = true;
			}
		}
		if (!found) {
			log("Debug: Playlist not found in setDrawnPlaylist()!");
			__debugbreak();
		}
	}
}

void core::MusicPlayer::setVolume(float volume)
{
	Mix_VolumeMusic(volume);
	this->volume = volume;
}

const core::MusicPlayer::MusicInfo& core::MusicPlayer::getPlayingMusicInfo() const
{
	/*
		playingOrder:               index in Playlist::musicIndexList
		Playlist::musicIndexList:   index in MusicPlayer::musicInfoList
		MusicPlayer::musicInfoList: music information
	*/
	return musicInfoList.at(getPlayingMusicIndex());
}

int core::MusicPlayer::getPlaylistPlayingMusicIndex() const
{
	return playingOrder.at(playingOrder_currentIndex);
}

int core::MusicPlayer::getPlayingMusicIndex() const
{
	return activePlaylist->musicIndexList.at(getPlaylistPlayingMusicIndex());
}

const core::Time core::MusicPlayer::getPlayingMusicElapsedTime() const
{
	return trackPlaytime.getElapsedTime();
}

std::string core::MusicPlayer::getActivePlaylistName() const
{
	if (activePlaylist) {
		return activePlaylist->name;
	}
	return "";
}

float core::MusicPlayer::getVolume() const
{
	return volume;
}

core::MusicPlayer::Replay core::MusicPlayer::getReplayStatus() const
{
	return replayStatus;
}

core::Time core::MusicPlayer::getPlaytime() const
{
	return playtime.getElapsedTime();
}

core::MusicPlayer::Report core::MusicPlayer::getSkipReport() const
{
	return skipReport;
}

core::MusicPlayer::Report core::MusicPlayer::getVolumeReport() const
{
	return volumeReport;
}

int core::MusicPlayer::getActivePlaylistSize() const
{
	if (activePlaylist) {
		return activePlaylist->musicIndexList.size();
	}
	return 0;
}

int core::MusicPlayer::getActivePlaylistCurrentTrackNumber() const
{
	if (activePlaylist) {
		return playingOrder_currentIndex + 1;
	}
	return 0;
}

core::Time core::MusicPlayer::getActivePlaylistDuration() const
{
	return activePlaylist->duration;
}

core::Time core::MusicPlayer::getActivePlaylistPlaytime() const
{
	return activePlaylist->oldTracksPlaytime + getPlayingMusicElapsedTime();
}

bool core::MusicPlayer::isPlaying() const
{
	return Mix_PlayingMusic() && !Mix_PausedMusic(); // SDL2 treats paused music as playing music
}

bool core::MusicPlayer::isPaused() const
{
	return Mix_PausedMusic();
}

bool core::MusicPlayer::isStopped() const
{
	return !Mix_PlayingMusic();
}

bool core::MusicPlayer::empty() const
{
	return musicInfoList.empty();
}

bool core::MusicPlayer::isShuffled() const
{
	return isShuffled_;
}

bool core::MusicPlayer::isTrappedOnTop() const
{
	if (drawnPlaylist) {
		return drawnPlaylist->drawableList.isTrappedOnTop();
	}
	return false;
}