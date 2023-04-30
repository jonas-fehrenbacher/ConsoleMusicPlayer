#include "core/SmallTools.hpp"
#include "core/ColoredStr.hpp"
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
	const char* fullBlock                     = u8"\u2588";
	const char* infinity                      = u8"\u221E";
	const char* latinSmallLetterOu            = u8"\u0223"; // shuffle
	const char* copticCapitalLetterGangia     = u8"\u03EA"; // shuffle
	const char* combiningCyrillicMillionsSign = u8"\u0489"; // shuffle
	const char* leftwardsArrow                = u8"\u2190";
	const char* blackRightPointingPointer     = u8"\u25BA"; // play
	const char* doubleVerticalLine            = u8"\u2016"; // pause
	const char* boxDrawingsDoubleVertical     = u8"\u2551"; // pause
	const char* latinLetterLateralClick       = u8"\u01C1"; // pause
	const char* blackSquare                   = u8"\u25A0"; // stop
	const char* rightwardsArrow               = u8"\u2192";
	const char* eighthNote                    = u8"\u266A";
	const char* beamedEighthNotes             = u8"\u266B";
	const char* modifierLetterUpArrowhead     = u8"\u02C4";
	const char* modifierLetterDownArrowhead   = u8"\u02C5";
	const char* overline                      = u8"\u203E";
	const char* ballotBoxWithCheck            = u8"\u2611";
	const char* ballotBox                     = u8"\u2610";
	const char* upwardsArrow                  = u8"\u2191";
	const char* downwardsArrow                = u8"\u2193";
}

void core::log(std::string message)
{
	std::ofstream ofs("data/log.txt", std::ios_base::app);
	ofs << message << "\n";
	ofs.close();
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

bool core::isSupportedAudioFile(fs::path filepath)
{
	static const std::vector<std::string> sdlSupportedExtentions{
		".flac", ".mp3", ".ogg", ".voc", ".wav", ".midi", ".mod", ".opus"
	};
	std::string audioFormat = filepath.extension().string();
	std::transform(audioFormat.begin(), audioFormat.end(), audioFormat.begin(),
		[](unsigned char c) { return std::tolower(c); }); // e.g. .WAV is also valid
	bool isSupported = false;
	for (auto& sdlSupportedExtention : sdlSupportedExtentions) {
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
	// Debug:
	return;

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
			std::cout << std::left << std::setw(4) << ColoredStr(std::to_string(fgcolor + bgcolor * 16), (Color)fgcolor, (Color)bgcolor);
		}
		std::cout << "\n";
	}
}