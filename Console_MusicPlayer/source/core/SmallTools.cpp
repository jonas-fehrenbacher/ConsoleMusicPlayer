#include "core/SmallTools.hpp"
#include "core/Console.hpp"
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <codecvt>
#include <locale>

namespace core::uc
{
	const char* fullBlock                       = u8"\u2588";
	const char* infinity                        = u8"\u221E";
	const char* latinSmallLetterOu              = u8"\u0223"; // shuffle
	const char* copticCapitalLetterGangia       = u8"\u03EA"; // shuffle
	const char* combiningCyrillicMillionsSign   = u8"\u0489"; // shuffle
	const char* leftwardsArrow                  = u8"\u2190";
	const char* blackRightPointingPointer       = u8"\u25BA"; // play
	const char* doubleVerticalLine              = u8"\u2016"; // pause
	const char* boxDrawingsDoubleVertical       = u8"\u2551"; // pause
	const char* latinLetterLateralClick         = u8"\u01C1"; // pause
	const char* blackSquare                     = u8"\u25A0"; // stop
	const char* rightwardsArrow                 = u8"\u2192";
	const char* eighthNote                      = u8"\u266A";
	const char* beamedEighthNotes               = u8"\u266B";
	const char* modifierLetterUpArrowhead       = u8"\u02C4";
	const char* modifierLetterDownArrowhead     = u8"\u02C5";
	const char* overline                        = u8"\u203E";
	const char* ballotBoxWithCheck              = u8"\u2611";
	const char* ballotBox                       = u8"\u2610";
	const char* upwardsArrow                    = u8"\u2191";
	const char* downwardsArrow                  = u8"\u2193";
	const char* blackUpPointingTriangle         = u8"\u25B2";
	const char* blackDownPointingTriangle       = u8"\u25BC";
	const char* blackRightPointingTriangle      = u8"\u25B6";
	const char* boxDrawingsLightHorizontal      = u8"\u2500";
	const char* boxDrawingsHeavyHorizontal      = u8"\u2501";
	const char* boxDrawingsLightVertical        = u8"\u2502";
	const char* boxDrawingsHeavyVertical        = u8"\u2503";
	const char* boxDrawingsLightDownAndRight    = u8"\u250C";
	const char* boxDrawingsHeavyDownAndRight    = u8"\u250F";
	const char* boxDrawingsLightDownAndLeft     = u8"\u2510";
	const char* boxDrawingsHeavyDownAndLeft     = u8"\u2513";
	const char* boxDrawingsLightUpAndRight      = u8"\u2514";
	const char* boxDrawingsHeavyUpAndRight      = u8"\u2517";
	const char* boxDrawingsLightUpAndLeft       = u8"\u2518";
	const char* boxDrawingsHeavyUpAndLeft       = u8"\u251B";
	const char* boxDrawingsLightArcDownAndRight = u8"\u256D";
	const char* boxDrawingsLightArcDownAndLeft  = u8"\u256E";
	const char* boxDrawingsLightArcUpAndLeft    = u8"\u256F";
	const char* boxDrawingsLightArcUpAndRight   = u8"\u2570";
}

void core::log(std::string message)
{
	std::ofstream ofs("data/log.txt", std::ios_base::app);
	ofs << message << "\n";
	ofs.close();
}

std::wstring core::toWStr(std::string str)
{
	//static std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> cv;
	static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> cv;
	return cv.from_bytes(str);
}

std::string core::toStr(std::wstring wstr)
{
	//static std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> cv;
	static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> cv;
	return cv.to_bytes(wstr);
}

std::string core::getUsername()
{
	return getenv("username");
}

long long core::getUUID()
{
	static long long id = 0;
	return id++;
}

std::string core::getTimeStr(core::Time time, core::Time limit /*= 0ns*/)
{
	if (limit == 0s && time == 0s) {
		return "0:00";
	}

	if (limit == 0s) {
		limit = time;
	}

	bool hasHours = false;
	bool hasMinutes = false;
	std::string timeStr = "";
	if (static_cast<int>(limit.asHours()) > 0) {
		int hours = (int)time.asHours();
		timeStr += std::to_string(hours) + ":";
		time -= core::Hours((int)time.asHours());
		hasHours = true;
	}
	if (static_cast<int>(limit.asMinutes()) > 0) {
		int minutes = (int)time.asMinutes();
		timeStr += (hasHours && minutes < 10 ? "0" : "") + std::to_string(minutes) + ":";
		time -= core::Minutes((int)time.asMinutes());
		hasMinutes = true;
	}
	if (static_cast<int>(limit.asSeconds()) > 0) { // note: don't do time > 0s, because an 1min song has at this point time==0
		int seconds = (int)time.asSeconds();
		timeStr += (hasMinutes && seconds < 10 ? "0" : "") + std::to_string(seconds);
		time -= core::Seconds((int)time.asSeconds());
	}
	return timeStr;
}

size_t core::getStrLength(const std::string& utf8)
{
	// see: https://stackoverflow.com/questions/31652407/how-to-get-the-accurate-length-of-a-stdstring

	//static std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
	//return conv.from_bytes(utf8).size();

	return (utf8.length() - std::count_if(utf8.begin(), utf8.end(), [](char c)->bool { return (c & 0xC0) == 0x80; })); // unicode chars start with 10xxxxxx in binary.
}

size_t core::getUniCodeCharCount(const std::string& utf8)
{
	// Can only count if string is u8""!
	return std::count_if(utf8.begin(), utf8.end(), [](char c)->bool { return (c & 0xC0) == 0x80; });

	//int count = 0;
	//const char* p = utf8.c_str();
	//while (*p != 0)
	//{
	//	if (!((*p & 0xc0) != 0x80))
	//		++count;
	//	++p;
	//}
	//return count;
}

size_t core::getUniCodeCharCount(const std::wstring& utf8)
{
	int count = 0;
	const wchar_t* p = utf8.c_str();
	while (*p != 0)
	{
		if (!((*p & 0xc0) != 0x80))
			++count;
		++p;
	}
	return count;
}

intern bool hasExtention(fs::path filepath, const std::vector<std::string>& extentions);

bool core::isSupportedAudioFile(fs::path filepath)
{
	static const std::vector<std::string> sdlSupportedExtentions{
		".flac", ".mp3", ".ogg", ".voc", ".wav", ".midi", ".mod", ".opus"
	};
	return hasExtention(filepath, sdlSupportedExtentions);
}

bool core::isAudioFile(fs::path filepath)
{
	static const std::vector<std::string> audioExtentions {
		".wv",	  // Format for wavpack files.
		".wma",	  // Microsoft	Windows Media Audio format, created by Microsoft. Designed with Digital Rights Management (DRM) abilities for copy protection.
		".webm",  // Royalty-free format created for HTML5 video.
		".wav",	  // Standard audio file container format used mainly in Windows PCs. Commonly used for storing uncompressed (PCM), CD-quality sound files, which means that they can be large in size—around 10 MB per minute. Wave files can also contain data encoded with a variety of (lossy) codecs to reduce the file size (for example the GSM or MP3 formats). Wav files use a RIFF structure.
		".vox",	  // The vox format most commonly uses the Dialogic ADPCM (Adaptive Differential Pulse Code Modulation) codec. Similar to other ADPCM formats, it compresses to 4-bits. Vox format files are similar to wave files except that the vox files contain no information about the file itself so the codec sample rate and number of channels must first be specified in order to play a vox file.
		".voc",	  // Creative Technology The file format consists of a 26-byte header and a series of subsequent data blocks containing the audio information
		".tta",	  // The True Audio, real-time lossless audio codec.
		".sln",	  // Signed Linear PCM format used by Asterisk. Prior to v.10 the standard formats were 16-bit Signed Linear PCM sampled at 8 kHz and at 16 kHz. With v.10 many more sampling rates were added.[7]
		".rf64",  // One successor to the Wav format, overcoming the 4GiB size limitation.
		".raw",	  // A raw file can contain audio in any format but is usually used with PCM audio data. It is rarely used except for technical tests.
		".ra", 	  // RealNetworks	A RealAudio format designed for streaming audio over the Internet. The .ra format allows files to be stored in a self-contained fashion on a computer, with all of the audio data contained inside the file itself.
		".rm",
		".opus",  // Internet Engineering Task Force	A lossy audio compression format developed by the Internet Engineering Task Force (IETF) and made especially suitable for interactive real-time applications over the Internet. As an open format standardised through RFC 6716, a reference implementation is provided under the 3-clause BSD license.
		".ogg",   // Xiph.Org Foundation	A free, open source container format supporting a variety of formats, the most popular of which is the audio format Vorbis. Vorbis offers compression similar to MP3 but is less popular. Mogg, the "Multi-Track-Single-Logical-Stream Ogg-Vorbis", is the multi-channel or multi-track Ogg file format.
		".oga",
		".mogg",
		".nmf",	  // NICE	NICE Media Player audio file
		".msv",	  // Sony	A Sony proprietary format for Memory Stick compressed voice files.
		".mpc",	  // Musepack or MPC (formerly known as MPEGplus, MPEG+ or MP+) is an open source lossy audio codec, specifically optimized for transparent compression of stereo audio at bitrates of 160–180 kbit/s.
		".mp3",	  // MPEG Layer III Audio. It is the most common sound file format used today.
		".mmf",	  // Yamaha, Samsung	A Samsung audio format that is used in ringtones. Developed by Yamaha (SMAF stands for "Synthetic music Mobile Application Format", and is a multimedia data format invented by the Yamaha Corporation, .mmf file format).
		".m4p",	  // Apple	A version of AAC with proprietary Digital Rights Management developed by Apple for use in music downloaded from their iTunes Music Store and their music streaming service known as Apple Music.
		".m4b",	  // An Audiobook / podcast extension with AAC or ALAC encoded audio in an MPEG-4 container. Both M4A and M4B formats can contain metadata including chapter markers, images, and hyperlinks, but M4B allows "bookmarks" (remembering the last listening spot), whereas M4A does not.[6]
		".m4a",	  // An audio-only MPEG-4 file, used by Apple for unprotected music downloaded from their iTunes Music Store. Audio within the m4a file is typically encoded with AAC, although lossless ALAC may also be used.
		".ivs",	  // 3D Solar UK Ltd	A proprietary version with Digital Rights Management developed by 3D Solar UK Ltd for use in music downloaded from their Tronme Music Store and interactive music and video player.
		".iklax", // iKlax	An iKlax Media proprietary format, the iKlax format is a multi-track digital audio format allowing various actions on musical data, for instance on mixing and volumes arrangements.
		".gsm",	  // Designed for telephony use in Europe, gsm is a very practical format for telephone quality voice. It makes a good compromise between file size and quality. Note that wav files can also be encoded with the gsm codec.
		".flac",  // A file format for the Free Lossless Audio Codec, an open-source lossless compression codec.
		".dvf",	  // Sony	A Sony proprietary format for compressed voice files; commonly used by Sony dictation recorders.
		".dss",	  // Olympus	DSS files are an Olympus proprietary format. It is a fairly old and poor codec. GSM or MP3 are generally preferred where the recorder allows. It allows additional data to be held in the file header.
		".cda",	  // Format for cda files for Radio.
		".awb",	  // AMR-WB audio, used primarily for speech, same as the ITU-T's G.722.2 specification.
		".au",	  // Sun Microsystems	The standard audio file format used by Sun, Unix and Java. The audio in au files can be PCM or compressed with the μ-law, a-law or G729 codecs.
		".ape",	  // Matthew T. Ashland	Monkey's Audio lossless audio compression format.
		".amr",	  // AMR-NB audio, used primarily for speech.
		".alac",  // Apple	An audio coding format developed by Apple Inc. for lossless data compression of digital music.
		".aiff",  // Apple	A standard uncompressed CD-quality, audio file format used by Apple. Established 3 years prior to Microsoft's uncompressed version wav.
		".act",	  // ACT is a lossy ADPCM 8 kbit/s compressed audio format recorded by most Chinese MP3 and MP4 players with a recording function, and voice recorders
		".aax",	  // Audible (Amazon.com)	An Audiobook format, which is a variable-bitrate (allowing high quality) M4B file encrypted with DRM. MPB contains AAC or ALAC encoded audio in an MPEG-4 container. (More details below.)
		".aac",	  // The Advanced Audio Coding format is based on the MPEG-2 and MPEG-4 standards. AAC files are usually ADTS or ADIF containers.
		".aa",    // Audible (Amazon.com)	A low-bitrate audiobook container format with DRM, containing audio encoded as either MP3 or the ACELP speech codec.
		".8svx",  // Electronic Arts	The IFF-8SVX format for 8-bit sound samples, created by Electronic Arts in 1984 at the birth of the Amiga.
		".3gp"	  // Multimedia container format can contain proprietary formats as AMR, AMR-WB or AMR-WB+, but also some open formats
		// see: https://en.wikipedia.org/wiki/Audio_file_format
	};
	return hasExtention(filepath, audioExtentions);
}

intern bool hasExtention(fs::path filepath, const std::vector<std::string>& extentions)
{
	std::string audioFormat = filepath.extension().string();
	std::transform(audioFormat.begin(), audioFormat.end(), audioFormat.begin(),
		[](unsigned char c) { return std::tolower(c); }); // e.g. .WAV is also valid
	bool isSupported = false;
	for (auto& sdlSupportedExtention : extentions) {
		if (audioFormat == sdlSupportedExtention) {
			isSupported = true;
			break;
		}
	}
	return isSupported;
}

bool core::hasFlag(int flag, int flagList)
{
	return flag == (flag & flagList);
}

void core::setConfig(std::filesystem::path path, const std::map<std::wstring, std::wstring>& config)
{
	std::wofstream ofs(path, std::ios::out);
	for (auto& [name, value] : config) {
		ofs << name << " = " << value << "\n";
	}
	ofs.close();
}

std::map<std::wstring, std::wstring> core::getConfig(std::filesystem::path path)
{
	std::map<std::wstring, std::wstring> config;
	std::wifstream ifs(path, std::ios::in);
	ifs.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));
	std::wstring line;

	while (std::getline(ifs, line))
	{
		// Note: Spaces may not be removed on 'line', because strings can contain spaces (e.g. paths).
		std::wstring varName = line.substr(0, line.find('='));
		// remove spaces:
		std::wstring::iterator newEndPos = std::remove(varName.begin(), varName.end(), ' ');
		varName.erase(newEndPos, varName.end());
		// Alternative works not for utf-8: varName.erase(std::remove_if(varName.begin(), varName.end(), isspace), varName.end());
		int varValueIndex = line.find('=') + 1;
		for (; line[varValueIndex] == ' '; ++varValueIndex);
		std::wstring varValue = line.substr(varValueIndex);
		config.insert(std::pair<std::wstring, std::wstring>(varName, varValue));
	}
	ifs.close();

	return config;
}

std::vector<std::wstring> core::getConfigStrArr(std::wstring strarr)
{
	// Example: musicDirs = "music", "C:/Users/Jonas/Music"
	std::vector<std::wstring> arr;
	for (int end = 0; strarr.find(L"\"", end + 1) != -1;) { // +1 because at the end I need to check for the next occurrence
		int begin = strarr.find(L"\"", end == 0 ? 0 : end + 1);
		end = strarr.find(L"\"", begin + 1);
		arr.push_back(strarr.substr(begin + 1, end - begin - 1));
	}
	return arr;
}

std::vector<fs::path> core::getConfigPathArr(std::wstring strarr)
{
	std::vector<fs::path> pathArr;
	std::vector<std::wstring> arr = getConfigStrArr(strarr);
	std::transform(arr.begin(), arr.end(), std::back_inserter(pathArr),
		[](const std::wstring& str) { return str; });
	return pathArr;
}

void core::testUCPrint()
{
	// 'Consolas' font special characters:
	std::cout << u8"\tendless:        \u221E\n";
	std::cout << u8"\tshuffle:        \u0223\n";
	std::cout << u8"\tshuffle:        \u03EA\n";
	std::cout << u8"\tshuffle:        \u0489\n";
	std::cout << u8"\tlinksPfeil:     \u2190\n";
	std::cout << u8"\tplay:           \u25BA\n";
	std::cout << u8"\tpause:          \u2016\n";
	std::cout << u8"\tpause:          \u2551\n";
	std::cout << u8"\tpause:          \u01C1\n"; // best
	std::cout << u8"\tstop:           \u25A0\n";
	std::cout << u8"\trechtsPfeil:    \u2192\n";
	std::cout << u8"\tnote:           \u266A\n";
	std::cout << u8"\tnote:           \u266B\n";
	std::cout << u8"\tscrollbarUp:    \u02C4\n";
	std::cout << u8"\tscrollbarDown:  \u02C5\n";
	std::cout << u8"\tüberstrich:     \u203E\n";
	std::cout << u8"\tcheckbox:       \u2611\n";
	std::cout << u8"\tbox:            \u2610\n";
	std::cout << u8"\tupwardsArrow:   \u2191\n";
	std::cout << u8"\tdownwardsArrow: \u2193\n";
}

void core::testColors()
{
	for (int bgcolor = 0; bgcolor < 16; ++bgcolor) {
		for (int fgcolor = 0; fgcolor < 16; ++fgcolor) {
			std::cout << std::left << std::setw(4) << Text(std::to_string(fgcolor + bgcolor * 16), (Color)fgcolor, (Color)bgcolor);
		}
		std::cout << "\n";
	}
}